#include "echo_myself.h"
#include <muduo/net/EventLoop.h>

int main()
{
	LOG_INFO << "pid is: " << getpid();
	muduo::net::EventLoop loop;
	muduo::net::InetAddress addr(2333);
	echo_myself bar(&loop, addr);
	bar.start();
	loop.loop();
	return 0;
}
