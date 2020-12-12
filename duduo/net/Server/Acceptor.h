#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <functional>
#include <muduo/net/InetAddress.h>
#include <memory>
#include <boost/noncopyable.hpp>

#include "../Reactor/Channel.h"
#include "Socket.h"

class EventLoop;

class Acceptor : boost::noncopyable
{
  // sending fd handle is not an ideal solution, better solution is 
  // sending a Socket object which uses RAII
  using ConnCallback = std::function<void (int, const muduo::net::InetAddress &)>;
  // using ConnCallback = std::function<void (Socket, const muduo::net::InetAddress &)>;
private:
  EventLoop *loop_;
  ConnCallback cb_;
  std::unique_ptr<Socket> socket_;
  Channel socketChannel_;
  bool listening_;

public:
  Acceptor(EventLoop *loop, const muduo::net::InetAddress & localAddr);
 ~Acceptor();

 bool isListening() const {
   return listening_;
 }

 void listen();
 void handleRead();
 void setNewConnectionCallback(const ConnCallback &func);
 
};

#endif /* ACCEPTOR_H */
