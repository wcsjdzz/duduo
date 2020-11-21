#include "EventLoop.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>

__thread EventLoop * loopOfCurrentThread_ = 0;

EventLoop::EventLoop():
  quit_(false),
  isLoopping_(false),
  threadId_(muduo::CurrentThread::tid()),
  maxWaitTimeM(10000),
  pollerPtr_(new Poller (this)),
  timerQueue_(new TimerQueue (this))
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
  quit_ = false;
  while(!quit_){
    activeChannels_.clear();
    pollerPtr_->poll(maxWaitTimeM, &activeChannels_);
    for(auto &ch: activeChannels_){
      ch->handleEvents();
    }
  }
  isLoopping_ = false;
  LOG_INFO << "EventLoop::loop() stopped";
}

TimerId EventLoop::runAt(const muduo::Timestamp &time, const TimerCallback &cb){
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb){
  muduo::Timestamp when(muduo::addTime(muduo::Timestamp::now(), delay));
  return runAt(when, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb){
  muduo::Timestamp time(muduo::Timestamp::now());
  return timerQueue_->addTimer(cb, time, interval);
}



void EventLoop::assertInLoopThread() {
  assert(threadId_ == muduo::CurrentThread::tid());
}

void EventLoop::updateChannel(Channel *ch){
  pollerPtr_->updateChannel(ch);
}

