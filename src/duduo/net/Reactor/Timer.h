#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <atomic>
#include <muduo/base/Timestamp.h>

class Timer
{
  using TimerCallback = std::function<void()>;
private:
  TimerCallback callback_;
  muduo::Timestamp expiration_;
  double interval_;
  bool repeat_;
  int64_t sequence_;
  static std::atomic<int64_t> sequenceNumGenerator_; // use std library atomic

public:
  Timer(const TimerCallback &cb, muduo::Timestamp when, double itv);
  ~Timer();
  
  muduo::Timestamp expiration() const {
    return expiration_;
  }
  double interval() const {
    return interval_;
  }
  int64_t sequence() const {
    return sequence_;
  }
  bool repeat() const {
    return repeat_;
  }

  void run() {
    callback_();
  }

  void restart(muduo::Timestamp when);
};

#endif /* TIMER_H */
