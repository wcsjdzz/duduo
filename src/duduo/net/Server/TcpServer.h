#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <memory>
#include <map>
#include <functional>
#include <string>
#include <muduo/net/InetAddress.h>
#include <boost/noncopyable.hpp>

#include "TcpConnection.h"

class Acceptor;
class EventLoop;

class TcpServer : boost::noncopyable
{
  using ConnectionCallback = std::function<void (const TcpConnectionPtr &)>;
  using MessageCallback = std::function<void (const TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp)>;
private:
  EventLoop *loop_;
  const std::string name_;
  muduo::net::InetAddress addr_;
  std::unique_ptr<Acceptor> acceptor_;
  std::map<std::string, TcpConnectionPtr> connections_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;

  static int connectionIndex_;

  // Only Acceptor could call the function when there is new tcp connection
  void onNewConnection(int peersockfd, const muduo::net::InetAddress &peeraddr);
  void removeConnection(const TcpConnectionPtr &);
  

public:
  TcpServer(EventLoop *loop, const muduo::net::InetAddress &addr);
  ~TcpServer();

  void start(); // user interface, enable the socket listening
  void setConnectionCallback(const ConnectionCallback &cb);
  void setMessageCallback(const MessageCallback &cb);
};

#endif /* TCPSERVER_H */
