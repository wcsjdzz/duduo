#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <boost/noncopyable.hpp>
#include <muduo/base/Timestamp.h>

// forward declaration, avoid dependency
class EventLoop;

// Dispatch of event
class Channel : boost::noncopyable
{
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void (muduo::Timestamp receiveTime)>;
private:
  EventLoop *ownerLoop_; // ownerLoop_ owns this Channel
  int fd_; // just as fd in struct pollfd
  int events_;
  int revents_;

  int index_; // necessary in Poller, -1 as inactivated

  bool isHandling_;

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventCallback errorCallback_;
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;

public:
  Channel(EventLoop *loop, int fd);
  ~Channel();

  int fd() const {
    return fd_;
  }
  int events() const {
    return events_;
  }
  int revents() const {
    return revents_;
  }
  int index() const {
    return index_;
  }

  bool isNoneEvent() const {
    return events_ == kNoneEvent;
  }
  bool isWriting() const{
    return events_ & kWriteEvent;
  }

  void set_index(int idx){
    index_ = idx;
  }
  void enableRead(){
    events_ = events_ | kReadEvent;
    update();
  }
  void enableWrite(){
    events_ = events_ | kWriteEvent;
    update();
  }
  void set_revent(int revents){
    revents_ = revents;
  }

  void disableAll();
  void disableWrite();

  void setErrorCallback(const EventCallback cb){
    errorCallback_ = cb;
  }
  void setReadCallback(const ReadEventCallback cb){
    readCallback_ = cb;
  }
  void setWriteCallback(const EventCallback cb){
    writeCallback_ = cb;
  }

  void handleEvents(const muduo::Timestamp &now);
  void update(); // Channel::update() ==> EventLoop::updateChannel()
  // ==> Poller::updateChannel()
  void remove();
};

#endif /* CHANNEL_H */
