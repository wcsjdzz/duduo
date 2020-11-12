#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>

#include <boost/any.hpp>

#include "muduo/base/Mutex.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/TimerId.h"
#include "muduo/base/Logging.h"


class EventLoop: muduo::noncopyable
{
private:
  void abortNotInLoopThread();
  bool isLoopping_;
  const pid_t threadId_;

public:
  EventLoop();
  ~EventLoop();

  bool isInLoopThread() const ;
  void assertInLoopThread();
  void loop();

  static EventLoop *getLoopInCurrentThread();
};

#endif /* EVENTLOOP_H */
