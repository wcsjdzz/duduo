#include "Timer.h"

std::atomic<int64_t> Timer::sequenceNumGenerator_(0);

Timer::Timer(const TimerCallback &cb, 
             muduo::Timestamp when,
             double itv) : 
  callback_(cb), expiration_(when), interval_(itv), repeat_(itv>0), 
  sequence_(sequenceNumGenerator_++) // atomic-operation
{

}

Timer::~Timer(){

}

void Timer::restart(muduo::Timestamp when){
  if(repeat_){
    expiration_ = muduo::addTime(when, interval_);
  } else {
    expiration_ = muduo::Timestamp::invalid();
  }
}
