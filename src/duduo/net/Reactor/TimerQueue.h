#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <muduo/base/Timestamp.h>
#include <set>
#include <vector>
#include <memory>

#include "Channel.h"

// forward declaration
class EventLoop;
class Timer;
class TimerId;

class TimerQueue
{
  using Entry = std::pair<muduo::Timestamp, Timer *>;
  using TimerList = std::set<Entry>;
  using TimerCallback = std::function<void()>;
private:
  EventLoop *loop_;
  const int timerfd_;
  TimerList timers_;
  Channel timerfdChannel;

  std::vector<Entry> getExpired(muduo::Timestamp now);
  

public:
  TimerQueue(EventLoop *);
  ~TimerQueue();

  TimerId addTimer(const TimerCallback cb, muduo::Timestamp time, double interval);

};

#endif /* TIMERQUEUE_H */
