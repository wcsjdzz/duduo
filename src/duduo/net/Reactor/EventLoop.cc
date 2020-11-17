#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>

__thread EventLoop * loopOfCurrentThread_ = 0;

EventLoop::EventLoop():
  pollerPtr_(new Poller (this)),
  quit_(false),
  isLoopping_(false),
  threadId_(muduo::CurrentThread::tid()),
  maxWaitTimeM(10000)
{
  if(loopOfCurrentThread_){
    LOG_FATAL << "Already has an EventLoop!";
  } else {
    loopOfCurrentThread_ = this;
  }
}

EventLoop::~EventLoop(){
  LOG_INFO << "EventLoop " << this << "deconstructed in thread " 
    << muduo::CurrentThread::tid();
  loopOfCurrentThread_ = 0;
}

void EventLoop::loop(){
  assert(!isLoopping_);
  assertInLoopThread();
  isLoopping_ = true;
  while(!quit_){
    activeChannels_.clear();
    pollerPtr_->poll(maxWaitTimeM, &activeChannels_);
    for(auto ch: activeChannels_){
      ch->handleEvents();
    }
  }
  isLoopping_ = false;
  LOG_INFO << "EventLoop::loop() stopped";
}

void EventLoop::assertInLoopThread() {
  assert(threadId_ == muduo::CurrentThread::tid());
}

void EventLoop::updateChannel(Channel *ch){
  pollerPtr_->updateChannel(ch);
}

