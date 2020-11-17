#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <memory>

class Channel;
class Poller;

// core of Reactor
class EventLoop
{
  using ChannelVec = std::vector<Channel *>;
private:
  ChannelVec activeChannels_;
  std::unique_ptr<Poller> pollerPtr_;
  bool quit_; // should be atomatic
  bool isLoopping_;
  const pid_t threadId_; // notes of IO thread
  int maxWaitTimeM;

  void assertInLoopThread();

public:
  EventLoop();
  ~EventLoop();

  // precondition:
  // 1. should be IO thread
  // 2. cannot call loop() repeately
  void loop();

  void updateChannel(Channel *ch);
  void quit() {
    quit_ =true;
  }
};

#endif /* EVENTLOOP_H */
