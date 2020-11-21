#include "Timer.h"

muduo::AtomicInt64 Timer::sequenceNumGenerator_;

Timer::Timer(const TimerCallback &cb, 
             muduo::Timestamp when,
             double itv) : 
  callback_(cb), expiration_(when), interval_(itv), repeat_(itv>0), 
  sequence_(sequenceNumGenerator_.incrementAndGet())
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
