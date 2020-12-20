#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <cstdint>
#include <string>
#include <chrono>

using namespace std::chrono;

// default: use microsecond resolution
template<typename ClockType, typename DuraType = typename ClockType::duration>
class TimestampBase {
  using TimepointType = time_point<ClockType, DuraType>;
 public:

  // Constucts an invalid TimestampBase.
  TimestampBase()
    : stamp_()
  {
  }

  // Constucts a TimestampBase at specific time
  // @param std::duration type to avoid ambiguity
  template<typename Duration2>
  explicit TimestampBase(Duration2 dura)
    : stamp_(dura)
  {
  }

  template<typename Duration2>
    explicit TimestampBase(const time_point<ClockType, Duration2> &t):
      stamp_(t)
  {
  }

  static const int kMicroSecondsPerSecond;

  void swap(TimestampBase& item)
  {
    using std::swap;
    swap(stamp_, item.stamp_);
  }

  // default copy/assignment/dtor are Okay

  std::string toString() const {
    char buf[32] = {0};
    auto microSecondsSinceEpoch = duration_cast<microseconds>(stamp_.time_since_epoch()).count();
    snprintf(buf, sizeof buf, "%ld %6ld", microSecondsSinceEpoch/kMicroSecondsPerSecond,
                                        microSecondsSinceEpoch%kMicroSecondsPerSecond);
    return buf;
  }
  std::string toFormattedString(bool showMicroseconds = true) const {
    // YYYY-MM-DD hh:mm:ss:micross
    char buf[64] = {0};
    auto microSecondsSinceEpoch = duration_cast<microseconds>(stamp_.time_since_epoch()).count();
    auto tt = system_clock::to_time_t(stamp_); // so this means that timestamp should use system_clock as ClockType
    auto tm_ = localtime(&tt);
    if(showMicroseconds){
      snprintf(buf, sizeof buf, "%4d-%02d-%02d %02d:%02d:%02d:%06ld", 
                                 tm_->tm_year+1900, tm_->tm_mon+1, tm_->tm_mday,
                                 tm_->tm_hour, tm_->tm_min, tm_->tm_sec,
                                 microSecondsSinceEpoch % kMicroSecondsPerSecond);
    } else {
      snprintf(buf, sizeof buf, "%4d-%02d-%02d %02d:%02d:%02d", 
                                 tm_->tm_year+1900, tm_->tm_mon+1, tm_->tm_mday,
                                 tm_->tm_hour, tm_->tm_min, tm_->tm_sec);
    }
    return buf;
  }

  bool valid() const { return stamp_ > TimepointType(); }

  // for internal usage.
  decltype(auto) microSecondsSinceEpoch() const { 
    return duration_cast<microseconds>(stamp_.time_since_epoch()).count(); 
  }
  decltype(auto) secondsSinceEpoch() const {
    return duration_cast<seconds>(stamp_.time_since_epoch()).count(); 
  }

  decltype(auto) getDuration() const {
    return stamp_.time_since_epoch();
  }

  // Get time of now.
  static TimestampBase now() {
    return TimestampBase(ClockType::now());
  }
  static TimestampBase invalid()
  {
    return TimestampBase(TimepointType());
  }

 private:
  TimepointType stamp_;
};

template<typename ClockType, typename DuraType >
const int TimestampBase<ClockType, DuraType>::kMicroSecondsPerSecond = 1000 * 1000;


using Timestamp = TimestampBase<system_clock>; // default type of timestamp

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microsecond
/// resolution for next 100 years.
inline decltype(auto) secondDifference(Timestamp high, Timestamp low)
{
  return duration_cast<seconds>(high.getDuration() - low.getDuration()).count();
}



#endif  // MUDUO_BASE_TIMESTAMP_H
