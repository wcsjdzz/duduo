#include "daytime_myself.h"
	void daytime_myself::onConnection(muduo::net::TcpConnectionPtr conn)
	{
		LOG_INFO << conn->peerAddress().toIpPort() << " -> " \
			<< conn->localAddress().toIpPort() << " is " \
			<< (conn->connected() ? "on" : "off");
		if(conn->connected())
		{
			conn->send(muduo::Timestamp::now().toFormattedString() + '\n');
			conn->shutdown();
		}
	}

	void daytime_myself::onMessage(muduo::net::TcpConnectionPtr conn, muduo::net::Buffer *buf, muduo::Timestamp time_)
	{
		muduo::string buf_(buf->retrieveAllAsString());
		LOG_INFO << conn->peerAddress().toIpPort() << " -> " \
			<< conn->localAddress().toIpPort() << " send " << buf_.size() << " data "
			<< " at time: " << time_.toFormattedString();
		conn->send(buf_);
	}
