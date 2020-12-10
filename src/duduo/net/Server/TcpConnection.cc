#include "TcpConnection.h"
#include "../Reactor/EventLoop.h"
#include "../Reactor/Channel.h"
#include "Socket.h"
#include <muduo/base/Logging.h>

TcpConnection::TcpConnection(EventLoop *loop, 
                const std::string &name,
                const muduo::net::InetAddress &local,
                const muduo::net::InetAddress &peer,
                int sockfd):
  loop_(loop),
  state_(kConnecting),
  name_(name),
  localAddr_(local),
  peerAddr_(peer),
  socket_(new Socket(sockfd)),
  sockChannel_(new Channel(loop, sockfd)),
  inputBuffer_(),
  outputBuffer_()
{
  sockChannel_->setReadCallback(
      std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)); // this! not shared_from_this
  sockChannel_->setWriteCallback(
      std::bind(&TcpConnection::handleWrite, this));
  // state_ = kConnected;
  sockChannel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
}

TcpConnection::~TcpConnection(){

  // connectionDestryed(); // call this function in the last line of dtor
  assert(state_ == kDisconnected);
}

void TcpConnection::setConnectionCallback(const ConnectionCallback &cb){
  connectionCallback_ = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback &cb){
  messageCallback_ = cb;
}

void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback &cb){
  writeCompleteCallback_ = cb;
}

void TcpConnection::setHighWaterCallback(const WriteCompleteCallback &cb){
  highWaterCallback_ = cb;
}

void TcpConnection::handleRead(muduo::Timestamp receiveTime){
  /* char buf[65536]; */
  /* int retSz = ::read(socket_->fd(), buf, sizeof buf); */
  loop_->assertInLoopThread(); // handle*() is called in IO thread, no doubt
  int errorFlag = 0;
  int retSz = inputBuffer_.readFd(socket_->fd(), &errorFlag);
  if(retSz > 0){
    // std::string msg(buf, retSz);
    // shared_from_this() makes TCP connection more safe
    LOG_INFO << "A new MessageCallback is comming";
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if(retSz == 0){
    handleClose();
  } else {
    // LOG_ERROR << "TcpConnection::handleRead() - error when calling read()";
    LOG_ERROR << "TcpConnection::handleRead() - errorFlag - " << errorFlag;
    handleError();
  }
}

void TcpConnection::handleWrite(){
  // handleWrite() should be called only when
  // msg couldn't be send() at the first time,
  // and the left msg is stored in outputBuffer_
  loop_->assertInLoopThread();
  if(!sockChannel_->isWriting()){
    LOG_TRACE << "TcpConnection::handleWrite - Connection is down";
  } else {
    size_t leftMsgSz = outputBuffer_.readableBytes();
    ssize_t ret = ::write(socket_->fd(), outputBuffer_.peek(), leftMsgSz);
    if(ret < 0){
      LOG_ERROR << "TcoConnection::handleWrite";
    }
    outputBuffer_.retrieve(ret);
    if(ret == leftMsgSz){ // or outputBuffer_.readableBytes() == 0
      sockChannel_->disableWrite();
      if(writeCompleteCallback_){
        loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_, shared_from_this()));
      }
      if(state_ == kDisconnecting){ // only when msg has send over then WR can be shut down
        shutdownInLoop();
      } 
    }
  }
}

void TcpConnection::handleClose(){
  loop_->assertInLoopThread();
  assert(state_ == kConnected);
  LOG_TRACE << "TcpConnection (" << name() << ") is being closed";
  // setState(kDisconnected); // state should be disconnected in connectionDestryed()
  sockChannel_->disableAll();
  closeCallback_(shared_from_this());
}

void TcpConnection::send(const std::string &msg){
  if(loop_->isInLoopThread()){
    sendInLoop(msg);
  } else {
    // shared_from_this shoud be better than `this`
    loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, shared_from_this(), msg));
  }
}

void TcpConnection::sendInLoop(const std::string &msg){
  loop_->assertInLoopThread();

  ssize_t ret = 0;
  ssize_t msgLen = static_cast<ssize_t>(msg.size());
  if(!sockChannel_->isWriting() && outputBuffer_.readableBytes() == 0){
    ret = ::write(socket_->fd(), msg.data(), static_cast<size_t>(msg.size()));
    if(ret < 0){
      LOG_ERROR << "TcpConnection::sendInLoop";
    }
  }

  assert(ret >= 0);
  if(ret < msgLen){
    outputBuffer_.append(msg.data()+ret, msgLen-ret);
    if(!sockChannel_->isWriting()){ // enable writable only when msg hasn't been send over 
      sockChannel_->enableWrite();
    }
  } else if(writeCompleteCallback_){
    loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_, shared_from_this()));
  }
}

void TcpConnection::shutdown(){
  if(loop_->isInLoopThread()){
    shutdownInLoop();
  } else {
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
  }
}

void TcpConnection::shutdownInLoop(){
  loop_->assertInLoopThread();
  if(!sockChannel_->isWriting()){
    socket_->shutdownWrite();
  }
}

void TcpConnection::handleError(){
  LOG_ERROR << name() << "TcpConnection::handleRead() - ::read() has error";
}

void TcpConnection::connectionEstablished(){
  loop_->assertInLoopThread();
  setState(kConnected);
  sockChannel_->enableRead(); // this is very important
  // once the connection is established, system should call this connection callback
  connectionCallback_(shared_from_this()); 
}

void TcpConnection::connectionDestryed(){
  loop_->assertInLoopThread();
  assert(state_ == kConnected);
  setState(kDisconnected);
  sockChannel_->disableAll();
  connectionCallback_(shared_from_this()); // why calling this callback?
  sockChannel_->remove(); // calling EventLoop->removeChannel()
}

void TcpConnection::setTcpNoDealy(bool on){
  socket_->setTcpNoDealy(on);
}

void TcpConnection::setKeepAlive(bool on){
  socket_->setKeepAlive(on);
}
