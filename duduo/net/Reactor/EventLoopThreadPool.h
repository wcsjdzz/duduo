#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <memory>
#include <vector>
#include <functional>

class EventLoop;
class EventLoopThread;

// EventLoopThreadPool - store thread resource
//                       and return usable EventLoop
class EventLoopThreadPool // noncopyable
{
  using ThreadInitCallback = std::function<void(EventLoop*)>;
private:
  EventLoopThreadPool(const EventLoopThreadPool &) = delete;
  EventLoopThreadPool &operator=(const EventLoopThreadPool &) = delete;

  EventLoop *baseLoop_;
  const std::string name_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_; // all loop in loops_ is ob-stack variable
  int numThreads_;
  int nextIndex_;
  bool started;


public:
  EventLoopThreadPool(EventLoop *baseLoop, const std::string &name);
  ~EventLoopThreadPool();

  // num: 0 - Server and Connection share the EventLoop
  //      1 - All Connections shared one thread(AKA IO thread)
  //      N - N IO thread
  void setThreadNums(int num);

  // create `numThreads` threads and start the main function of every thread
  // should be called in base loop, which is the thread of SERVER
  void start(const ThreadInitCallback &cb = ThreadInitCallback());

  // get the next loop in loops_ as IO loop
  EventLoop *getNextLoop();
};

#endif /* EVENTLOOPTHREADPOOL_H */
