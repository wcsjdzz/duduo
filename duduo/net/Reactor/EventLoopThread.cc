#include "EventLoopThread.h"
#include "EventLoop.h"

void EventLoopThread::threadFunc(){
  EventLoop loop; // use on-stack-variable
  if(initCallback_){
    initCallback_(&loop);
  }
  {
    std::lock_guard<std::mutex> currGuard(mutex_);
    loop_ = &loop; // other thread waits for non-empty loop_
  }
  condition_.notify_all(); // ok, loop created, notify `startLoop()`
  loop.loop();
}

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, 
                                 const std::string &name):
  name_(name),
  initCallback_(cb),
  loop_(nullptr),
  mutex_(),
  condition_()
{

}

EventLoopThread::~EventLoopThread()
{

}

EventLoop *EventLoopThread::startLoop(){
  std::thread IOthread(std::bind(&EventLoopThread::threadFunc, this));
  IOthread.detach();
  {
    std::unique_lock<std::mutex> lk(mutex_);
    condition_.wait(lk, [this](){return loop_;});
  }
  return loop_;
}
