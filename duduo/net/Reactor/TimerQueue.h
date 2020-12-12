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

class TimerQueue // noncopyable
{
  using Entry = std::pair<muduo::Timestamp, Timer *>;
  using TimerList = std::set<Entry>;
  using TimerCallback = std::function<void()>;
private:
  TimerQueue (const TimerQueue &) = delete;
  TimerQueue &operator=(const TimerQueue &) = delete;

  EventLoop *loop_;
  const int timerfd_;
  TimerList timers_;
  Channel timerfdChannel_;

  std::vector<Entry> getExpired(muduo::Timestamp now);
  bool insert(Timer *);
  void resetTimerfd(muduo::Timestamp when);
  

public:
  TimerQueue(EventLoop *);
  ~TimerQueue();

  TimerId addTimer(const TimerCallback cb, muduo::Timestamp time, double interval);
  void addTimerInLoop(Timer *timer);
  void handleRead();
  void reset(std::vector<Entry> &expired, muduo::Timestamp now);

};

#endif /* TIMERQUEUE_H */
