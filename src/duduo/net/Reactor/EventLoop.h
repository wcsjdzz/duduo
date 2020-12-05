#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <muduo/base/Timestamp.h>
#include <muduo/base/Mutex.h>

#include <vector>
#include <memory>
#include <functional>

class Channel;
class Poller;
class TimerQueue;
class TimerId;


// core of Reactor
class EventLoop
{
  using Callback = std::function<void()>;
  using ChannelVec = std::vector<Channel *>;
private:
  ChannelVec activeChannels_;
  bool quit_; // should be atomatic
  bool isLoopping_;
  const pid_t threadId_; // notes of IO thread
  int maxWaitTimeM;
  std::unique_ptr<Poller> pollerPtr_;
  std::unique_ptr<TimerQueue> timerQueue_;


  int wakeupFd_; // wake up IO thread which is blocked in IO multiplexing
  std::unique_ptr<Channel> wakeupChannel_;
  void handleRead();

  std::vector<Callback> funcQueue; // store the callback function in queue
  bool isExecuteQueueFunc;

  void executeQueueFunctions();

  muduo::MutexLock mutex_;

public:
  EventLoop();
  ~EventLoop();

  bool isInLoopThread();
  void assertInLoopThread();

  // precondition:
  // 1. should be IO thread
  // 2. cannot call loop() repeately
  void loop();
  void queueInLoop(const Callback &cb);

  TimerId runAt(const muduo::Timestamp &time, const Callback &cb);
  TimerId runAfter(double delay, const Callback &cb);
  TimerId runEvery(double interval, const Callback &cb);

  void wakeup(); // let IO thread wake up in poll()
  void runInLoop(const Callback &cb);

  void updateChannel(Channel *ch);
  void quit();
  void removeChannel(Channel *ch);
};

#endif /* EVENTLOOP_H */
