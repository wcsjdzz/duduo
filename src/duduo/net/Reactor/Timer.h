#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Atomic.h>

class Timer
{
  using TimerCallback = std::function<void()>;
private:
  TimerCallback callback_;
  muduo::Timestamp expiration_;
  double interval_;
  bool repeat_;
  int64_t sequence_;
  static muduo::AtomicInt64 sequenceNumGenerator_;

public:
  Timer(const TimerCallback &cb, muduo::Timestamp when, double itv);
  ~Timer();
  
  muduo::Timestamp expiration() const {
    return expiration_;
  }
  double interval() {
    return interval_;
  }
  int64_t sequence() const {
    return sequence_;
  }

  void run() {
    callback_();
  }

  void restart(muduo::Timestamp when);
};

#endif /* TIMER_H */
