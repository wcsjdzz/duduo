#include "EventLoop.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>
#include <signal.h>
#include <sys/eventfd.h>

__thread EventLoop * loopOfCurrentThread_ = 0;

// ignore SIGPIPE to avoid exiting progress unexpectedly
struct ignSigPipe{
  ignSigPipe(){
    ::signal(SIGPIPE, SIG_IGN);
  }
};
ignSigPipe ignSigPipe_;

int creatEventFd(){
  return ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC); // FIXME - 0 or other?
}

EventLoop::EventLoop():
  quit_(false),
  isLoopping_(false),
  threadId_(muduo::CurrentThread::tid()),
  maxWaitTimeM(10000),
  pollerPtr_(std::make_unique<Poller>(this)),
  timerQueue_(std::make_unique<TimerQueue>(this)),
  wakeupFd_(creatEventFd()),
  wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
  funcQueue(),
  isExecuteQueueFunc(false),
  mutex_()
{
  if(loopOfCurrentThread_){
    LOG_FATAL << "EventLoop::EventLoop() - Already has an EventLoop!";
  } else {
    loopOfCurrentThread_ = this;
  }
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableRead();
}

EventLoop::~EventLoop(){
  LOG_INFO << "EventLoop " << this << " deconstructed in thread " 
    << muduo::CurrentThread::tid();
  loopOfCurrentThread_ = 0;
  ::close(wakeupFd_); // who handles this fd, who close it
}

void EventLoop::loop(){
  assert(!isLoopping_);
  assertInLoopThread();
  isLoopping_ = true;
  quit_ = false;
  while(!quit_){
    activeChannels_.clear();
    auto receiveTime = pollerPtr_->poll(maxWaitTimeM, &activeChannels_);
    for(auto &ch: activeChannels_){
      ch->handleEvents(receiveTime);
    }
    executeQueueFunctions();
  }
  isLoopping_ = false;
  LOG_INFO << "EventLoop::loop() - stopped looping";
}

TimerId EventLoop::runAt(const muduo::Timestamp &time, const Callback &cb){
  //printf("Starting addTimer\n");
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const Callback &cb){
  muduo::Timestamp when(muduo::addTime(muduo::Timestamp::now(), delay));
  return runAt(when, cb);
}

TimerId EventLoop::runEvery(double interval, const Callback &cb){
  muduo::Timestamp time(muduo::Timestamp::now());
  return timerQueue_->addTimer(cb, time, interval);
}

bool EventLoop::isInLoopThread(){
  return threadId_ == muduo::CurrentThread::tid();
}

void EventLoop::assertInLoopThread() {
  assert(isInLoopThread());
}

void EventLoop::updateChannel(Channel *ch){
  pollerPtr_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel *ch){
  assertInLoopThread();
  pollerPtr_->removeChannel(ch);
}

void EventLoop::wakeup(){
  int64_t value = 0; // send a meaningless value
  if(write(wakeupFd_, &value, sizeof value) < 0){
    LOG_ERROR << "EventLoop::wakeup() - WRONG in ::write";
  }
}

void EventLoop::handleRead(){
  int64_t value = 0;
  if(read(wakeupFd_, &value, sizeof value) < 0){
    LOG_ERROR << "EventLoop::handleRead() - WRONG in ::read";
  }
}

void EventLoop::runInLoop(const Callback &cb){
  if(isInLoopThread()){
    cb();
  } else {
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(const Callback &cb){
  {
    std::lock_guard<std::mutex> guard(mutex_);
    funcQueue.push_back(cb);
  }
  if(!isInLoopThread() || isExecuteQueueFunc){
    // no need to wakeup the IOthread when IOthread is 
    // calling this function out of executeQueueFunctions(), 
    // because executeQueueFunctions must be called soon
    wakeup();
  }
}

void EventLoop::executeQueueFunctions(){
  isExecuteQueueFunc = true;
  std::vector<Callback> tmp;
  {
    using std::swap; // this is the standard usage in c++ primer
    std::lock_guard<std::mutex> guard(mutex_);
    swap(funcQueue, tmp);
  }
  for(const auto &func: tmp){
    func();
  }
  isExecuteQueueFunc = false;
}

void EventLoop::quit() {
  quit_ = true;
  if(!isInLoopThread()){
    // at other thread, so IO thread might be blocked in IO multiplexing
    wakeup();
  } // otherwise EventLoop should quit the loop soon
  LOG_INFO << "EventLoop::quit() - quit the loop";
}
