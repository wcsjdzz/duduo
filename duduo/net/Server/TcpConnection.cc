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
  state_(connState::kConnecting),
  name_(name),
  localAddr_(local),
  peerAddr_(peer),
  socket_(std::make_unique<Socket>(sockfd)),
  sockChannel_(std::make_unique<Channel>(loop, sockfd)),
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
  assert(state_ == connState::kDisconnected);
}

void TcpConnection::setConnectionCallback(const ConnectionCallback &cb){
  connectionCallback_ = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback &cb){
  messageCallback_ = cb;
}

void TcpConnection::setCloseCallback(const CloseCallback &cb){
  closeCallback_ = cb;
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
    LOG_TRACE << "TcpConnection::handleWrite() - have shutdown the write endian";
  } else {
    ssize_t leftMsgSz = outputBuffer_.readableBytes();
    ssize_t ret = ::write(socket_->fd(), outputBuffer_.peek(), leftMsgSz);
    if(ret < 0){
      LOG_ERROR << "TcoConnection::handleWrite";
    }
    outputBuffer_.retrieve(ret);
    if(ret == leftMsgSz){ // or outputBuffer_.readableBytes() == 0
      sockChannel_->disableWrite(); // Level Trigger, must disable write after writing
      if(writeCompleteCallback_){
        loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_, shared_from_this()));
      }
      if(state_ == connState::kDisconnecting){ // only when msg has send over then WR can be shut down
        shutdownInLoop();
      } 
    }
  }
}

void TcpConnection::handleClose(){
  loop_->assertInLoopThread();
  assert(state_ == connState::kConnected || state_ == connState::kDisconnecting);
  LOG_TRACE << "TcpConnection (" << name() << ") is being closed";
  setState(connState::kDisconnected); // state should be disconnected in connectionDestryed()
  sockChannel_->disableAll();
  closeCallback_(shared_from_this()); // closeCallback_ is binded to TcpServer::removeChannel
}

void TcpConnection::send(const std::string &msg){
  if(state_ == connState::kConnected){
    if(loop_->isInLoopThread()){
      sendInLoop(msg);
    } else {
      // shared_from_this shoud be better than `this`
      loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, shared_from_this(), msg));
    }
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

void TcpConnection::shutdown(){ // send `FIN` - shutdown() here is used to keep the half-connection status
  if(state_ == connState::kConnected){
    setState(connState::kDisconnecting);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
  }
}

void TcpConnection::shutdownInLoop(){
  loop_->assertInLoopThread();
  if(!sockChannel_->isWriting()){
    socket_->shutdownWrite();
  }
  LOG_INFO << "TCP connection status is " << static_cast<int>(state_);
}

void TcpConnection::handleError(){
  LOG_ERROR << name() << "TcpConnection::handleRead() - ::read() has error";
}

void TcpConnection::connectionEstablished(){
  loop_->assertInLoopThread();
  setState(connState::kConnected);
  sockChannel_->enableRead(); // this is very important
  // once the connection is established, system should call this connection callback
  connectionCallback_(shared_from_this()); 
}

void TcpConnection::connectionDestryed(){
  loop_->assertInLoopThread();
  if(state_ == connState::kConnected){ // means `connectionDestryed` could be called without calling `handleClose`
    setState(connState::kDisconnected);
    sockChannel_->disableAll();
  }
  sockChannel_->remove(); // calling EventLoop->removeChannel()
}

void TcpConnection::setTcpNoDealy(bool on){
  socket_->setTcpNoDealy(on);
}

void TcpConnection::setKeepAlive(bool on){
  socket_->setKeepAlive(on);
}
