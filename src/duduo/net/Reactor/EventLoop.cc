#include "EventLoop.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>
#include <sys/eventfd.h>

__thread EventLoop * loopOfCurrentThread_ = 0;

int creatEventFd(){
  return ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC); // FIXME - 0 or other?
}

EventLoop::EventLoop():
  quit_(false),
  isLoopping_(false),
  threadId_(muduo::CurrentThread::tid()),
  maxWaitTimeM(10000),
  pollerPtr_(new Poller (this)),
  timerQueue_(new TimerQueue (this)),
  wakeupFd_(creatEventFd()),
  wakeupChannel_(new Channel(this, wakeupFd_)),
  funcQueue(),
  isExecuteQueueFunc(false),
  mutex_()
{
  if(loopOfCurrentThread_){
    LOG_FATAL << "Already has an EventLoop!";
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
  ::close(wakeupFd_); // should close this eventfd
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
  LOG_INFO << "EventLoop::loop() stopped";
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
  //printf("starting wakeup()\n");
  int64_t value = 0; // send this meaningless value
  write(wakeupFd_, &value, sizeof value);
  //printf("wakeup() done\n");
}

void EventLoop::handleRead(){
  int64_t value = 0;
  read(wakeupFd_, &value, sizeof value);
}

void EventLoop::runInLoop(const Callback &cb){
  if(isInLoopThread()){
    //printf("runInLoop() - is in IO thread\n");
    cb();
  } else {
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(const Callback &cb){
  {
    muduo::MutexLockGuard guard(mutex_);
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
    muduo::MutexLockGuard guard(mutex_);
    std::swap(funcQueue, tmp);
  }
  for(const auto &func: tmp){
    func();
  }
  isExecuteQueueFunc = false;
}

void EventLoop::quit() {
  //printf("starting quit()\n");
  LOG_INFO << "EventLoop::quit() - staring";
  quit_ = true;
  //printf("quit_ becomes true\n");
  if(!isInLoopThread()){
    // at other thread, so IO thread might be blocked in IO multiplexing
    //printf("test -- not in the IO thread\n");
    wakeup();
  }
  LOG_INFO << "EventLoop::quit() - done";
  //printf("quit() done\n");
}
