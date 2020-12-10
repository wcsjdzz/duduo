#include "Channel.h"
#include "EventLoop.h"
#include <muduo/base/Logging.h>
#include <poll.h>

// static variable should have a initialization
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd):
  ownerLoop_(loop),
  fd_(fd),
  events_(kNoneEvent),
  revents_(0),
  index_(-1), 
  isHandling_(false)
{

}

// dtor of Channel does not close file descriptor
Channel::~Channel() {
  // must not be handing event when destructing channel
  assert(!isHandling_);
}

void Channel::handleEvents(const muduo::Timestamp &now) {
  isHandling_ = true;
  if(revents_ & (POLLNVAL|POLLERR)){
    if(errorCallback_) errorCallback_();
  }
  if( (revents_&POLLHUP) && !(revents_&POLLIN) ){
    LOG_WARN << "Channel - handling POLLHUP event";
    if(closeCallback_) closeCallback_();
  }
  if(revents_ & (POLLIN | POLLPRI | POLLRDHUP)){
    if(readCallback_) readCallback_(now);
  }
  if(revents_ & POLLOUT){
    if(writeCallback_) writeCallback_();
  }
  isHandling_ = false;
}

void Channel::update(){
  ownerLoop_->updateChannel(this);
}

void Channel::remove(){
  assert(isNoneEvent());
  ownerLoop_->removeChannel(this);
}

void Channel::disableAll(){
  events_ = kNoneEvent;
  update();
}

void Channel::disableWrite(){
  events_ = events_ & (~kWriteEvent);
}
