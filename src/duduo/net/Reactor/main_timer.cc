#include "EventLoop.h"
#include "TimerId.h"

EventLoop *g_loop;

void timeOutFunc(){
  printf("Oooops, time out!!!\n");
  g_loop->quit();
}

int main(int argc, char *argv[])
{
  EventLoop loop;
  loop.runAfter(5, &timeOutFunc);
  loop.loop();
  return 0;
}
