#include <muduo/base/Logging.h>

#include "../Reactor/EventLoop.h"
#include "../Reactor/Channel.h"
#include "Acceptor.h"
#include "SockOptions.h"
#include "Socket.h"

Acceptor::Acceptor(EventLoop *loop, const muduo::net::InetAddress &localAddr)
  :loop_(loop),
  socket_(new Socket (sockoptions::createNonblockingOrDie(AF_INET))),
  socketChannel_(loop, socket_->fd()),
  listening_(false)
{
  socket_->setReuseAddr(true);
  socket_->bindAddress(localAddr);
  socketChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{

}

void Acceptor::listen(){
  loop_->assertInLoopThread();
  listening_ = true;
  socket_->listen(); // call the socket API `listen()`
  socketChannel_.enableRead(); // ready to call the callback
}

void Acceptor::handleRead(){
  muduo::net::InetAddress addr; // this is the perr address
  int connfd = socket_->accept(&addr); // connected fd
  if(connfd < 0){
    LOG_FATAL << "Acceptor - socket accept failed";
    return ;
  }
  if(cb_){
    cb_(connfd, addr);
  } else {
    LOG_ERROR << "Acceptor - NewConnectionCallback unset";
  }
}

void Acceptor::setNewConnectionCallback(const ConnCallback &func){
  cb_ = func;
}
