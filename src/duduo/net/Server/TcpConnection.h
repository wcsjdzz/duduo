#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <memory>
#include <functional>
#include <boost/noncopyable.hpp>
#include <muduo/net/InetAddress.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>

class EventLoop;
class Socket;
class Channel;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

/* Attribute of TcpConnection */
// 1. connection status: connecting, connected
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
class TcpConnection : boost::noncopyable, 
  public std::enable_shared_from_this<TcpConnection>
{
  enum StateEnum {kConnected, kConnecting, kDisconnected, kDisconnecting};
  using ConnectionCallback = std::function<void (const TcpConnectionPtr &)>;
  // using MessageCallback = std::function<void (const TcpConnectionPtr &, const std::string &)>;
  using MessageCallback = std::function<void (const TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp)>;
  using CloseCallback = std::function<void (const TcpConnectionPtr &)>;
  using WriteCompleteCallback = std::function<void (const TcpConnectionPtr &)>;
  using HighWaterCallback = std::function<void (const TcpConnectionPtr &)>;
private:
  EventLoop *loop_;
  StateEnum state_;
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
  CloseCallback closeCallback_;
  WriteCompleteCallback writeCompleteCallback_; // called when outputBuffer_ is empty after `::write`
  HighWaterCallback highWaterCallback_;

  void shutdown();
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
  void setCloseCallback(const CloseCallback &cb)
  {
    closeCallback_ = cb;
  }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb);
  void setHighWaterCallback(const WriteCompleteCallback &cb);

  void setState(const StateEnum &st){
    state_ = st;
  }
  void setTcpNoDealy(bool on);
  void setKeepAlive(bool on);

  void connectionEstablished();
  void connectionDestryed();

  void send(const std::string &msg);
  void sendInLoop(const std::string &msg);
};

#endif /* TCPCONNECTION_H */
