#include <sys/timerfd.h>
#include "TimerQueue.h"
#include "Channel.h"
#include "Timer.h"
#include "EventLoop.h"
#include "TimerId.h"

int creatTimerFd(){
  return ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(muduo::Timestamp now){
  Entry dummy{now, reinterpret_cast<Timer *>(UINTPTR_MAX)};
  auto end_ = timers_.lower_bound(dummy);
  assert(end_==timers_.end() || now<end_->first);
  auto ret = std::vector<Entry>(timers_.begin(), end_);
  timers_.erase(timers_.begin(), end_);
  return ret;
}

TimerId TimerQueue::addTimer(const TimerCallback cb,
                          muduo::Timestamp time,
                          double interval){
  auto pir = timers_.insert({time, new Timer(cb, time, interval)});
  return TimerId(pir.first->second, pir.first->second->sequence());

}

TimerQueue::TimerQueue(EventLoop *loop):
  loop_(loop),
  timerfd_(creatTimerFd()),
  timers_(),
  timerfdChannel(loop, timerfd_)
{

}

TimerQueue::~TimerQueue(){
  ::close(timerfd_);
  for(auto itr = timers_.begin(); itr != timers_.end(); ++itr){
    delete itr->second;
  }
}
