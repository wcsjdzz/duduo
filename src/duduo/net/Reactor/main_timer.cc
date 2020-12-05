#include "EventLoop.h"
#include <muduo/base/Thread.h>
#include "TimerId.h"

EventLoop *g_loop;

void timeOutFunc(){
  printf("Oooops, time out!!!\n");
  g_loop->quit();
}

void threadFunc(){
  g_loop->runAfter(15, &timeOutFunc);
}

int main(int argc, char *argv[])
{
  EventLoop loop;
  g_loop = &loop;
  muduo::Thread thread_(&threadFunc);
  thread_.start();
  loop.loop();
  return 0;
}
