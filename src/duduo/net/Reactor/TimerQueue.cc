#include <sys/timerfd.h>
#include <unistd.h>
#include "TimerQueue.h"
#include "Channel.h"
#include "Timer.h"
#include "EventLoop.h"
#include "TimerId.h"

int creatTimerFd(){
  return ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
}

struct timespec getTimeDiffFromNow(muduo::Timestamp when){
  auto microSeconds = when.microSecondsSinceEpoch() 
                      - muduo::Timestamp::now().microSecondsSinceEpoch();
  if(microSeconds < 100)
    microSeconds = 100; // set the minimal diff as 100 micro seconds
  struct timespec ret;
  // seconds
  ret.tv_sec = static_cast<time_t>(microSeconds / muduo::Timestamp::kMicroSecondsPerSecond);
  // nano seconds
  ret.tv_nsec = static_cast<__SYSCALL_SLONG_TYPE>( (microSeconds%muduo::Timestamp::kMicroSecondsPerSecond) * 1000 );
  return ret;
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
  Timer *timer = new Timer(cb, time, interval);
  loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
  return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer *timer){
  loop_->assertInLoopThread();
  auto isEarliest = insert(timer);
  if(isEarliest){
    resetTimerfd(timer->expiration());
    //printf("addtimer done\n");
  }
}

TimerQueue::TimerQueue(EventLoop *loop):
  loop_(loop),
  timerfd_(creatTimerFd()),
  timers_(),
  timerfdChannel_(loop, timerfd_)
{
  timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
  timerfdChannel_.enableRead(); // update timerfd Channel to Poller
  //printf("::TimerQueue done\n");
}

TimerQueue::~TimerQueue(){
  ::close(timerfd_);
  for(auto itr = timers_.begin(); itr != timers_.end(); ++itr){
    delete itr->second;
  }
}

void TimerQueue::handleRead(){
  loop_->assertInLoopThread();
  muduo::Timestamp now(muduo::Timestamp::now());
  auto expiredList = getExpired(now);
  for(auto &expired: expiredList){
    expired.second->run();
  }
  //printf("runned expired timer\n");
  reset(expiredList, now);
  //printf("reset done\n");
}

void TimerQueue::reset(std::vector<Entry> &expired, muduo::Timestamp now){
  muduo::Timestamp when;
  for(auto &e: expired){
    if(e.second->repeat()){
      e.second->restart(now); // addTimer(now, interval);
      insert(e.second);
    }
  }
  if(!timers_.empty()){
    when = timers_.begin()->first;
  } else {
    when = muduo::Timestamp::invalid();
  }
  if(when.valid()){
    resetTimerfd(when); // set the timer only when timers_ is not empty
    // and the next alarm time is the first element of timers_
  }
}

bool TimerQueue::insert(Timer * timer){
  bool isEarliset = false;
  auto itr = timers_.begin();
  if(itr==timers_.end() || timer->expiration()<itr->first){
    isEarliset = true;
  }
  timers_.insert({timer->expiration(), timer});
  return isEarliset;
}

void TimerQueue::resetTimerfd(muduo::Timestamp when){
  itimerspec old_timerspec, new_timerspec;
  bzero(&old_timerspec, sizeof old_timerspec);
  bzero(&new_timerspec, sizeof new_timerspec);
  new_timerspec.it_value = getTimeDiffFromNow(when);
  //printf("timer diff from now is %ld\n", new_timerspec.it_value.tv_sec);
  ::timerfd_settime(timerfd_, 0, &new_timerspec, &old_timerspec);;
}
