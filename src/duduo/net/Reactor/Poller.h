#ifndef POLLER_H
#define POLLER_H

#include <vector>
#include <map>
#include <muduo/base/Timestamp.h>

class Channel;
class EventLoop;
struct pollfd;

// Poller - wrapped API associated with IO multiplexing
class Poller // noncopyable
{
  using ChannelVec = std::vector<Channel *>;
  using ChannelMap = std::map<int, Channel *>; // fd => Channel *
private:
  Poller(const Poller &) = delete;
  Poller &operator=(const Poller &) = delete;

  std::vector<struct pollfd> pollfds_;
  ChannelMap channels_;
  EventLoop *ownerLoop_;

public:
  Poller(EventLoop *loop);
  ~Poller();

  void updateChannel(Channel *); // add pollfd in pollfds_
  void removeChannel(Channel *); // remove pollfd in pollfds_
  muduo::Timestamp poll(int maxWaitTimeM, ChannelVec *activeChannels);
  void fillActiveChannels(int activeNum, ChannelVec *activeChannels);
};

#endif /* POLLER_H */
