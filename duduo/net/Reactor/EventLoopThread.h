#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class EventLoop;

class EventLoopThread
{
  using ThreadInitCallback = std::function<void(EventLoop *)>;
private:
  const std::string name_;
  ThreadInitCallback initCallback_; // called when 
                                    // on-stack-EventLoop is established
  EventLoop *loop_;
  std::mutex mutex_;
  std::condition_variable condition_;
  void threadFunc(); // main function of current thread

public:
  EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name= std::string());
  ~EventLoopThread();

  // create new thread in `statrLoop`
  // executing the main function `threadFunc` 
  // and return corresponding EventLoop
  EventLoop *startLoop();
};

#endif /* EVENTLOOPTHREAD_H */
