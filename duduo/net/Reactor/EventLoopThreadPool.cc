#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &name)
  : baseLoop_(baseLoop),
    name_(name),
    numThreads_(0),
    nextIndex_(0),
    started(false)
{

}

EventLoopThreadPool::~EventLoopThreadPool()
{

}

void EventLoopThreadPool::setThreadNums(int num){
  assert(num >= 0);
  numThreads_ = num;
}

EventLoop *EventLoopThreadPool::getNextLoop(){
  assert(started);
  baseLoop_->assertInLoopThread();
  if(numThreads_ == 0)  return baseLoop_;
  auto ret =  loops_[nextIndex_];
  nextIndex_ = (nextIndex_ + 1) % loops_.size();
  return ret;
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb ){
  assert(!started);
  baseLoop_->assertInLoopThread();
  started = true;
  for(int i = 0; i < numThreads_; ++i){
    const auto threadName = name_ + std::to_string(i);
    threads_.push_back(std::make_unique<EventLoopThread>(cb, threadName));
    loops_.push_back(threads_.back()->startLoop());
  }
  if(numThreads_==0 && cb){
    // if numThreads_ is not zero,
    // then the init callback would be called in the main function of new thread
    cb(baseLoop_);
  }
}
