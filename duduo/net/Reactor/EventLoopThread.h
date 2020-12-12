#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <muduo/base/Thread.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Condition.h>

class EventLoop;

class EventLoopThread
{
private:
  EventLoop *loop_;
  muduo::Thread thread_;
  muduo::MutexLock mutex_;
  muduo::Condition condition_;
  void threadFunc(); // main function of current thread

public:
  EventLoopThread();
  ~EventLoopThread();

  EventLoop *startLoop();
};

#endif /* EVENTLOOPTHREAD_H */
