#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <memory>
#include <functional>
#include <muduo/net/InetAddress.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>

class EventLoop;
class Socket;
class Channel;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

/* Attribute of TcpConnection */
// 1. connection status: connecting, connected, disconenecting, disconnected
// 
// 2. name: each server could have multiple 
//          TCP connection, so a name is identical as a flag
//
// 3. address: include local address and peer address
//
// 4. socket: to handle readable and writable event, this could
//            be solved by Channel and Socket

// due to fuzzy life cycle of TCP connection,
// using std::shared_ptr is the best method to
// achieve thread safety
class TcpConnection : // noncopyable
  public std::enable_shared_from_this<TcpConnection>
{
  // kDisconnecting - LIKE POLLRDHUP, the write endian is closed - half-close status
  enum class connState {kConnecting, kConnected, kDisconnecting, kDisconnected};
  // using ConnectionCallback = std::function<void (const TcpConnectionPtr &)>;
  // // using MessageCallback = std::function<void (const TcpConnectionPtr &, const std::string &)>;
  // using MessageCallback = std::function<void (const TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp)>;
  // using CloseCallback = std::function<void (const TcpConnectionPtr &)>;
  // using WriteCompleteCallback = std::function<void (const TcpConnectionPtr &)>;
  // using HighWaterCallback = std::function<void (const TcpConnectionPtr &)>;
  template<typename ...T>
    using Callback = std::function<void(const TcpConnectionPtr &, T...)>;
  using ConnectionCallback = Callback<>;
  using MessageCallback = Callback<muduo::net::Buffer *, muduo::Timestamp>;
  using CloseCallback = Callback<>;
  using WriteCompleteCallback = Callback<>;
  using HighWaterCallback = Callback<>;
private:
  TcpConnection (const TcpConnection &) = delete;
  TcpConnection &operator=(const TcpConnection &) = delete;

  EventLoop *loop_;
  connState state_;
  const std::string name_;
  muduo::net::InetAddress localAddr_, peerAddr_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> sockChannel_;
  muduo::net::Buffer inputBuffer_; // for handleRead
  muduo::net::Buffer outputBuffer_; // for handleWrite


  // TcpConnection handle the readable event by itself,
  // once the msg is recieved already, it sends msg to messageCallback
  void handleRead(muduo::Timestamp reveiveTime); 
  void handleWrite();
  void handleError();
  void handleClose();
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  CloseCallback closeCallback_; // closeCallback_ is binded to TcpServer::removeChannel
  WriteCompleteCallback writeCompleteCallback_; // called when outputBuffer_ is empty after `::write`
  HighWaterCallback highWaterCallback_;

  void shutdownInLoop();


public:
  TcpConnection(EventLoop *loop, 
                const std::string &name,
                const muduo::net::InetAddress &local,
                const muduo::net::InetAddress &peer,
                int sockfd);
  ~TcpConnection(); // TcpConnection should close the peer sockfd when connection shut down

  const std::string & name() const {
    return name_;
  }

  const muduo::net::InetAddress & peerAddr() const {
    return peerAddr_;
  }

  void setConnectionCallback(const ConnectionCallback &cb);
  void setMessageCallback(const MessageCallback &cb);
  void setCloseCallback(const CloseCallback &cb); // used by client and server, not the normal user of net library
  void setWriteCompleteCallback(const WriteCompleteCallback &cb);
  void setHighWaterCallback(const WriteCompleteCallback &cb);

  void setState(const connState &st){
    state_ = st;
  }
  void setTcpNoDealy(bool on);
  void setKeepAlive(bool on);

  void connectionEstablished();
  void connectionDestryed(); // 1. last function called before dtor. 
                             // 2. could be called without handleClose
                             // 3. called in TcpServer::removeConnection

  void send(const std::string &msg);
  void sendInLoop(const std::string &msg);

  void shutdown(); // shutdown the write edian of TCP connection, 
                   // meaning TCP server cannot send data to client anymore, 
                   // but the connection is still alive

  EventLoop *getLoop() {
    return loop_;
  }
};

#endif /* TCPCONNECTION_H */
