#include "TcpServer.h"
#include "../Reactor/EventLoop.h"
#include "../Reactor/EventLoopThread.h"
#include "../Reactor/EventLoopThreadPool.h"
#include "Acceptor.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <atomic>

using std::placeholders::_1;
using std::placeholders::_2;

std::atomic<int64_t> serverIndexGen(0);

std::string serverDefaultName(){
  return "TcpServer" + std::to_string(serverIndexGen++);
}

int TcpServer::connectionIndex_ = 0;

TcpServer::TcpServer(EventLoop *loop, 
                     const muduo::net::InetAddress &addr):
  loop_(loop),
  name_(serverDefaultName()),
  addr_(addr),
  acceptor_(std::make_unique<Acceptor>(loop, addr)),
  connections_(),
  threadPool_(std::make_unique<EventLoopThreadPool>(loop, name_)),
  started(0)
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

void TcpServer::setThreadInitCallback(const ThreadInitCallback &cb){
  threadInitCallback_ = cb;
}

void TcpServer::onNewConnection(int peersockfd, const muduo::net::InetAddress &peeraddr){
  LOG_INFO << "[" << name_ << "]" << " - new connection comming from - "
           << peeraddr.toIpPort();
  std::string connName = name_ + std::to_string(connectionIndex_++);
  auto IOloop = threadPool_->getNextLoop();
  TcpConnectionPtr newConn(
      std::make_shared<TcpConnection>(
        IOloop, connName, addr_, peeraddr, peersockfd));
  connections_[connName] = newConn;
  newConn->setConnectionCallback(connectionCallback_);
  newConn->setMessageCallback(messageCallback_);
  newConn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
  newConn->setWriteCompleteCallback(writeCompleteCallback_);
  // newConn->connectionEstablished();
  // better way is to make connectionEstablished run in IO thread
  IOloop->runInLoop(std::bind(&TcpConnection::connectionEstablished, newConn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn){
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn){
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnectionInLoop() - "
           << "remove " << conn->name() << " from " << name_;
  auto erasedNum = connections_.erase(conn->name());
  assert(erasedNum == 1); // should have and only have one corresponding loop
  conn->getLoop()->queueInLoop(std::bind(&TcpConnection::connectionDestryed, conn));
}

void TcpServer::start(){
  if(started++ == 0){
    threadPool_->start(threadInitCallback_);
    assert(!acceptor_->isListening());
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

void TcpServer::setThreadNum(int numThread){
  assert(numThread>=0);
  threadPool_->setThreadNums(numThread);
}
