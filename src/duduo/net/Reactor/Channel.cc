#include "Channel.h"
#include "EventLoop.h"
#include <poll.h>

// static variable should have a initialization
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd):
  ownerLoop_(loop),
  fd_(fd),
  events_(0),
  revents_(0),
  index_(-1)
{

}

void Channel::handleEvents() {
  if(revents_ & (POLLNVAL|POLLERR)){
    printf("error event comes\n");
    if(errorCallback_) errorCallback_();
  }
  if(revents_ & (POLLIN | POLLPRI | POLLRDHUP)){
    printf("read event comes\n");
    if(readCallback_) readCallback_();
  }
  if(revents_ & POLLOUT){
    printf("write event comes\n");
    if(writeCallback_) writeCallback_();
  }
}

void Channel::update(){
  ownerLoop_->updateChannel(this);
}
