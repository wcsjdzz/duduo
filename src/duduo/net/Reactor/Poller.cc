#include <assert.h>
#include <poll.h>
#include "Poller.h"
#include "Channel.h"
#include <muduo/base/Logging.h>

Poller::Poller(EventLoop *loop): ownerLoop_(loop) {

}

Poller::~Poller() {

}

void Poller::updateChannel(Channel *ch){
  if(ch->index() < 0){ // this channel is a new one
    assert(channels_.find(ch->fd()) == channels_.end());
    struct pollfd tmp;
    tmp.events = ch->events();
    tmp.revents = ch->revents();
    tmp.fd = ch->fd();
    pollfds_.push_back(tmp);
    channels_[tmp.fd] = ch;
    ch->set_index(pollfds_.size()-1);
  } else { // it alreay stores in pollfds_, we just change the value
    assert(channels_.find(ch->fd()) != channels_.end());
    struct pollfd &tmp = pollfds_[ch->index()];
    tmp.events = ch->events();
    tmp.revents = ch->revents();
    if(ch->isNoneEvent()){
      tmp.fd = -1; // no event under watched, so set -1
    }
  }
}

void Poller::poll(int maxWaitTimeM, ChannelVec *activeChannels){
  int activeNum = ::poll(pollfds_.data(), pollfds_.size(), maxWaitTimeM);
  if(activeNum>0){
    fillActiveChannels(activeNum, activeChannels);
  } else if(!activeNum){
    LOG_INFO << "No active event in " << maxWaitTimeM << " m seconds";
  } else {
    LOG_FATAL << "ERROR occurs when ::poll()";
  }
}

void Poller::fillActiveChannels(int activeNum, ChannelVec *activeChannels){
  for(const auto &tmp: pollfds_){
    if(!activeNum)  break;
    if(tmp.revents){
      --activeNum;
      activeChannels->push_back(channels_[tmp.fd]);
    }
  }
}
