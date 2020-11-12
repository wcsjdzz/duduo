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


class EventLoop: muduo::noncopyable
{
private:
  void abortNotInLoopThread();
  bool isLoopping;
  const pid_t threadId_;

public:
  EventLoop();
  virtual ~EventLoop();

  bool isInLoopThread() const ;
  void assertInLoopThread();
  void loop();
};

#endif /* EVENTLOOP_H */
