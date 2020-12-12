#include "TcpServer.h"
#include "../Reactor/EventLoop.h"
#include "Acceptor.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>

using std::placeholders::_1;
using std::placeholders::_2;

std::string serverDefaultName(){
  return "TcpServer" + muduo::Timestamp::now().toString();
}

int TcpServer::connectionIndex_ = 0;

TcpServer::TcpServer(EventLoop *loop, 
                     const muduo::net::InetAddress &addr):
  loop_(loop),
  name_(serverDefaultName()),
  addr_(addr),
  acceptor_(new Acceptor (loop, addr)),
  connections_()
{
  acceptor_->setNewConnectionCallback(
      std::bind(&TcpServer::onNewConnection, this, _1, _2));
}

TcpServer::~TcpServer(){ // Server should close its TCP connection
  loop_->assertInLoopThread();
  for(auto &conn: connections_){
    auto tmp = conn.second; // use local stack variable is more safe
    conn.second.reset();
    tmp->getLoop()->runInLoop(std::bind(&TcpConnection::connectionDestryed, tmp));
  }
}

void TcpServer::setMessageCallback(const MessageCallback &cb){
  messageCallback_ = cb;
}

void TcpServer::setConnectionCallback(const ConnectionCallback &cb){
  connectionCallback_ = cb;
}

void TcpServer::onNewConnection(int peersockfd, const muduo::net::InetAddress &peeraddr){
  LOG_INFO << "TcpServer - new connection comming from - "
           << peeraddr.toIpPort();
  std::string connName = name_ + std::to_string(connectionIndex_);
  TcpConnectionPtr newConn(
      std::make_shared<TcpConnection>(
        loop_, connName, addr_, peeraddr, peersockfd));
  connections_[connName] = newConn;
  newConn->setConnectionCallback(connectionCallback_);
  newConn->setMessageCallback(messageCallback_);
  newConn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
  newConn->setWriteCompleteCallback(writeCompleteCallback_);
  // newConn->connectionEstablished();
  // better way is to make connectionEstablished run in IO thread
  loop_->runInLoop(std::bind(&TcpConnection::connectionEstablished, newConn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn){
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnection " << conn->name()
           << " from " << name_;
  auto erasedNum = connections_.erase(conn->name());
  assert(erasedNum == 1); // should have and only have one corresponding loop
  loop_->queueInLoop(std::bind(&TcpConnection::connectionDestryed, conn));
}

void TcpServer::start(){
  acceptor_->listen();
}
