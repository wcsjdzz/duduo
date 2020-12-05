#include <assert.h>
#include <poll.h>
#include "Poller.h"
#include "Channel.h"
#include "EventLoop.h"
#include <muduo/base/Logging.h>

Poller::Poller(EventLoop *loop): ownerLoop_(loop) {

}

Poller::~Poller() {

}

void Poller::updateChannel(Channel *ch){
  //printf("Starting updateChannel\n");
  if(ch->index() < 0){ // this channel is a new one
    assert(channels_.find(ch->fd()) == channels_.end());
    struct pollfd tmp;
    tmp.events = ch->events();
    tmp.revents = 0;
    tmp.fd = ch->fd();
    //printf("the fd of updated Channel is %d\n", tmp.fd);
    pollfds_.push_back(tmp);
    channels_[tmp.fd] = ch;
    ch->set_index(pollfds_.size()-1);
  } else { // it alreay stores in pollfds_, we just change the value
    assert(channels_.find(ch->fd()) != channels_.end());
    struct pollfd &tmp = pollfds_[ch->index()];
    // in pollfds_, every fd should have only 2 state: fd || -fd - 1
    // -fd - 1 tells system that this file descriptor is unwatched while
    //         keeping its origing value
    assert(tmp.fd == ch->fd() || tmp.fd == -ch->fd() - 1);
    tmp.events = ch->events();
    tmp.revents = 0;
    if(ch->isNoneEvent()){
      tmp.fd = -ch->fd() - 1; // no event under watched, so set -1
    }
  }
  //printf("updateChannel done, now the size of pollfds_ is %lu\n", pollfds_.size());
}

void Poller::removeChannel(Channel *ch){
  // if ch is at the end of channel_ exactly, pop the pollfd vector 
  // if not, exchange elements and pop vector

  ownerLoop_->assertInLoopThread();

  // channels_
  auto fd = ch->fd();
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[ch->fd()] == ch);
  assert(ch->isNoneEvent()); // when remoce channel, is should disable all listening events

  // pollfds_
  auto index = ch->index();
  assert(0<=index && static_cast<size_t>(index)<pollfds_.size());
  int num = channels_.erase(fd); // erase from map
  assert(num == 1);
  // -fd-1 means it is a unwatched fd
  assert(pollfds_[index].fd == -ch->fd()-1 && pollfds_[index].events==ch->events());
  if(static_cast<size_t> (index) == pollfds_.size()-1){ // erase from vector<struct pollerfd>
    pollfds_.pop_back();
  } else {
    auto endFd = pollfds_.back().fd;
    if(endFd < 0) endFd = -endFd - 1; // to transfer it to the exact file descriptor
    assert(channels_.find(endFd) != channels_.end());
    std::swap(pollfds_[index], pollfds_.back());
    channels_[endFd]->set_index(index);
    pollfds_.pop_back();
  }
}

muduo::Timestamp Poller::poll(int maxWaitTimeM, ChannelVec *activeChannels){
  int activeNum = ::poll(pollfds_.data(), pollfds_.size(), maxWaitTimeM);
  muduo::Timestamp reveiveTime(muduo::Timestamp::now());
  if(activeNum>0){
    fillActiveChannels(activeNum, activeChannels);
  } else if(!activeNum){
    LOG_INFO << "No active event in " << maxWaitTimeM << " m seconds";
  } else {
    LOG_FATAL << "ERROR occurs when ::poll()";
  }
  return reveiveTime;
}

void Poller::fillActiveChannels(int activeNum, ChannelVec *activeChannels){
  for(const auto &tmp: pollfds_){
    if(activeNum<=0)  break;
    if(tmp.revents>0){
      assert(channels_.find(tmp.fd) != channels_.end());
      --activeNum;
      channels_[tmp.fd]->set_revent(tmp.revents); // revent of channel should be updated
      activeChannels->push_back(channels_[tmp.fd]);
    }
  }
}
