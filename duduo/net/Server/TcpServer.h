#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <atomic>
#include <memory>
#include <map>
#include <functional>
#include <string>
#include <muduo/net/InetAddress.h>

#include "TcpConnection.h"

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer // noncopyable
{
  template<typename ...T>
    using Callback = std::function<void(const TcpConnectionPtr &, T...)>;
  using ConnectionCallback = Callback<>;
  using MessageCallback = Callback<muduo::net::Buffer *, muduo::Timestamp>;
  using WriteCompleteCallback = Callback<>;
  using HighWaterCallback = Callback<>;
  using ThreadInitCallback = std::function<void(EventLoop *)>;
private:
  TcpServer(const TcpServer &) = delete;
  TcpServer &operator=(const TcpServer &) = delete;

  EventLoop *loop_;
  const std::string name_;
  muduo::net::InetAddress addr_;
  std::unique_ptr<Acceptor> acceptor_;
  std::map<std::string, TcpConnectionPtr> connections_;
  std::unique_ptr<EventLoopThreadPool> threadPool_;
  std::atomic<int> started;

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_; // called when outputBuffer_ is empty after `::write`
  HighWaterCallback highWaterCallback_;
  ThreadInitCallback threadInitCallback_;

  static int connectionIndex_;

  // Only Acceptor could call the function when there is new tcp connection
  void onNewConnection(int peersockfd, const muduo::net::InetAddress &peeraddr);
  void removeConnection(const TcpConnectionPtr &);
  void removeConnectionInLoop(const TcpConnectionPtr &);
  

public:
  TcpServer(EventLoop *loop, const muduo::net::InetAddress &addr);
  ~TcpServer();

  void start(); // user interface, enable the socket listening
  void setConnectionCallback(const ConnectionCallback &cb);
  void setMessageCallback(const MessageCallback &cb);
  void setWriteCompleteCallback(const WriteCompleteCallback &cb);
  void setThreadInitCallback(const ThreadInitCallback &cb);

  // set the thread number in threadPool_
  void setThreadNum(int numThread);
};

#endif /* TCPSERVER_H */
