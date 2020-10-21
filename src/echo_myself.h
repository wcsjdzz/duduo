#ifndef ECHO_MYSELF

#define ECHO_MYSELF
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
/*! \class echo_myself
*  \brief a simple echo server
*
*  just send back all the receiced data
*/
class echo_myself
{
public:
	// loop: eventloop, a reactor;
	// addr: internet address that this server should listen;
	echo_myself(muduo::net::EventLoop *loop, const muduo::net::InetAddress &addr)
		: loop_(loop),
		  server_(loop, addr, "EchoServer")
	{
		server_.setConnectionCallback(std::bind(&echo_myself::onConnection, this, _1));
		server_.setMessageCallback(std::bind(&echo_myself::onMessage, this, _1, _2, _3));
	}

	void start()
	{
		server_.start();
	}

protected:
	muduo::net::EventLoop *loop_; // we need event loop to define a reactor;
	muduo::net::TcpServer server_; // use tcp server to define a echo server;

	// callback:
	void onConnection(muduo::net::TcpConnectionPtr conn);
	void onMessage(muduo::net::TcpConnectionPtr conn, muduo::net::Buffer *buf, muduo::Timestamp time_);
};

#endif
