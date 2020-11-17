#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>

// forward declaration, avoid dependency
class EventLoop;

// Dispatch of event
class Channel
{
  using eventCallback = std::function<void()>;
private:
  EventLoop *ownerLoop_; // ownerLoop_ owns this Channel
  int fd_; // just as fd in struct pollfd
  int events_;
  int revents_;

  int index_; // necessary in Poller, -1 as inactivated

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  eventCallback errorCallback_;
  eventCallback readCallback_;
  eventCallback writeCallback_;

public:
  Channel(EventLoop *loop, int fd);
  ~Channel()
  {

  }

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
    revents = revents;
    update();
  }

  void setErrorCallback(const eventCallback cb){
    errorCallback_ = cb;
  }
  void setReadCallback(const eventCallback cb){
    readCallback_ = cb;
  }
  void setWriteCallback(const eventCallback cb){
    writeCallback_ = cb;
  }

  void handleEvents();
  void update(); // Channel::update() ==> EventLoop::updateChannel()
  // ==> Poller::updateChannel()
};

#endif /* CHANNEL_H */
