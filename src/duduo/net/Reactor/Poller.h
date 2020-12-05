#ifndef POLLER_H
#define POLLER_H

#include <vector>
#include <map>
#include <muduo/base/noncopyable.h>
#include <muduo/base/Timestamp.h>

class Channel;
class EventLoop;
struct pollfd;

class Poller : muduo::noncopyable
{
  using ChannelVec = std::vector<Channel *>;
  using ChannelMap = std::map<int, Channel *>; // fd => Channel *
private:
  std::vector<struct pollfd> pollfds_;
  ChannelMap channels_;
  EventLoop *ownerLoop_;

public:
  Poller(EventLoop *loop);
  ~Poller();

  void updateChannel(Channel *);
  void removeChannel(Channel *);
  muduo::Timestamp poll(int maxWaitTimeM, ChannelVec *activeChannels);
  void fillActiveChannels(int activeNum, ChannelVec *activeChannels);
};

#endif /* POLLER_H */
