#include "echo_myself.h"
#include "daytime_myself.h"
#include <muduo/net/EventLoop.h>

int main()
{
	LOG_INFO << "pid is: " << getpid();
	muduo::net::EventLoop loop;

	echo_myself echo_server(&loop, muduo::net::InetAddress(2333));
	echo_server.start();

	daytime_myself daytime_server(&loop, muduo::net::InetAddress(3333));
	daytime_server.start();

	loop.loop(); // as a reactor for multiple servers, but only one thread;
	return 0;
}
