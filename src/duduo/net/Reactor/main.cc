#include <cstring>
#include <sys/timerfd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "EventLoop.h"
#include "Channel.h"

EventLoop *g_loop = NULL;

void timeoutFunc(){
  printf("Time Out!\n");
  g_loop->quit();
}

void errorFunc(){
  printf("error occurs while poll()\n");
  g_loop->quit();
}

void writeFunc(){
  printf("write write\n");
  g_loop->quit();
}

int main(int argc, char *argv[])
{
  EventLoop loop;
  g_loop = &loop;
  int tfd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  Channel ch(&loop, tfd_);
  ch.setReadCallback(&timeoutFunc);
  ch.setErrorCallback(&errorFunc);
  ch.setWriteCallback(&writeFunc);
  ch.enableRead();

  struct itimerspec timer;
  bzero(&timer, sizeof timer);
  timer.it_value.tv_sec = 5;
  ::timerfd_settime(tfd_, 0, &timer, NULL);
  loop.loop();
  ::close(tfd_);
  return 0;
}
