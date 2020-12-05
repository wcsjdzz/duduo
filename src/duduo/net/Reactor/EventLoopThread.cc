#include "EventLoopThread.h"
#include "EventLoop.h"

void EventLoopThread::threadFunc(){
  EventLoop loop;
  {
    muduo::MutexLockGuard guard_(mutex_);
    loop_ = &loop;
    condition_.notify();
  }
  loop.loop();
}

EventLoopThread::EventLoopThread():
  loop_(nullptr),
  thread_(std::bind(&EventLoopThread::threadFunc, this)),
  mutex_(),
  condition_(mutex_)
{

}

EventLoopThread::~EventLoopThread()
{

}

EventLoop *EventLoopThread::startLoop(){
  assert(thread_.started());
  thread_.start();
  {
    muduo::MutexLockGuard guard_(mutex_);
    while(loop_ == nullptr){
      condition_.wait();
    }
  }
  return loop_;
}
