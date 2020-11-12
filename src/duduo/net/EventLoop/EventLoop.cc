#include "EventLoop.h"

// indicate corresponding Eventloop object of current thread
__thread EventLoop *t_loopInthisThread = 0;

EventLoop::EventLoop():
  isLoopping_(false),
  threadId_(muduo::CurrentThread::tid())
{
  LOG_TRACE << "tid " << muduo::CurrentThread::tid()
    << " created EventLoop " << this;
  if(t_loopInthisThread){
    LOG_FATAL << "current thread already has a EventLoop!"; // exit
  } else {
    t_loopInthisThread = this;
  }
}

EventLoop::~EventLoop(){
  assert(!isLoopping_);
  t_loopInthisThread = 0; // return the flag variable
}


