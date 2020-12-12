#ifndef DAYTIME_MYSELF

#define DAYTIME_MYSELF
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
/*! \class daytime_myself
*  \brief a simple daytime server
*
*  just send back all the receiced data
*/
class daytime_myself
{
public:
	// loop: eventloop, a reactor;
	// addr: internet address that this server should listen;
	daytime_myself(muduo::net::EventLoop *loop, const muduo::net::InetAddress &addr)
		: loop_(loop),
		  server_(loop, addr, "daytimeServer")
	{
		server_.setConnectionCallback(std::bind(&daytime_myself::onConnection, this, _1));
	}

	void start()
	{
		server_.start();
	}

protected:
	muduo::net::EventLoop *loop_; // we need event loop to define a reactor;
	muduo::net::TcpServer server_; // use tcp server to define a daytime server;

	// callback:
	void onConnection(muduo::net::TcpConnectionPtr conn);
	void onMessage(muduo::net::TcpConnectionPtr conn, muduo::net::Buffer *buf, muduo::Timestamp time_);
};

#endif
