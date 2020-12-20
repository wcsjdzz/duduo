#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <muduo/base/Timestamp.h>

// forward declaration, avoid dependency
class EventLoop;

// Channel - Dispatch of event
//           manage file descriptor and corresponding event callback
class Channel // noncopyable
{
  template<typename ...T>
    using Callback = std::function<void(T...)>;
  using EventCallback = Callback<>;
  using WriteCallback = Callback<>;
  using ErrorCallback = Callback<>;
  using CloseCallback = Callback<>;
  using ReadEventCallback = Callback<muduo::Timestamp>;
private:
  Channel(const Channel &) = delete;
  Channel &operator=(const Channel &) = delete;

  EventLoop *ownerLoop_;
  int fd_;
  int events_;
  int revents_;

  int index_; // `ownerLoop_->Poller->pollfds_[Channel::index_].fd` is equal to `Channel::fd_`;

  bool isHandling_; // indicate whether this Channel is handling event

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventCallback errorCallback_;
  ReadEventCallback readCallback_;
  WriteCallback writeCallback_;
  CloseCallback closeCallback_;

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

  void setErrorCallback(const ErrorCallback cb){
    errorCallback_ = cb;
  }
  void setReadCallback(const ReadEventCallback cb){
    readCallback_ = cb;
  }
  void setWriteCallback(const WriteCallback cb){
    writeCallback_ = cb;
  }
  void setCloseCallback(const CloseCallback cb){
    closeCallback_ = cb;
  }

  void handleEvents(const muduo::Timestamp &now);
  void update(); // Channel::update() ==> EventLoop::updateChannel() ==> update pollfds_ in EventLoop::Poller
  void remove(); // remove pollfds_ in EventLoop::Poller
};

#endif /* CHANNEL_H */
