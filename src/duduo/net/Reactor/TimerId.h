#ifndef TIMERID_H
#define TIMERID_H

#include <cstdint>

class Timer;

class TimerId
{
private:
  Timer *timer_;
  int64_t sequence_;

public:
  TimerId():
    timer_(nullptr),
    sequence_(0){

  }
  TimerId(Timer *t, int64_t sequ):
    timer_(t), sequence_(sequ) {

  }
};

#endif /* TIMERID_H */
