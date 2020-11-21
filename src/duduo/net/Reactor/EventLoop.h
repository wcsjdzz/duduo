#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <muduo/base/Timestamp.h>

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
  using TimerCallback = std::function<void()>;
  using ChannelVec = std::vector<Channel *>;
private:
  ChannelVec activeChannels_;
  bool quit_; // should be atomatic
  bool isLoopping_;
  const pid_t threadId_; // notes of IO thread
  int maxWaitTimeM;
  std::unique_ptr<Poller> pollerPtr_;
  std::unique_ptr<TimerQueue> timerQueue_;

  void assertInLoopThread();

public:
  EventLoop();
  ~EventLoop();

  // precondition:
  // 1. should be IO thread
  // 2. cannot call loop() repeately
  void loop();

  TimerId runAt(const muduo::Timestamp &time, const TimerCallback &cb);
  TimerId runAfter(double delay, const TimerCallback &cb);
  TimerId runEvery(double interval, const TimerCallback &cb);


  void updateChannel(Channel *ch);
  void quit() {
    quit_ =true;
  }
};

#endif /* EVENTLOOP_H */
