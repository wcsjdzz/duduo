#include "echo_myself.h"
	void echo_myself::onConnection(muduo::net::TcpConnectionPtr conn)
	{
		LOG_INFO << conn->peerAddress().toIpPort() << " -> " \
			<< conn->localAddress().toIpPort() << " is " \
			<< (conn->connected() ? "on" : "off");
	}

	void echo_myself::onMessage(muduo::net::TcpConnectionPtr conn, muduo::net::Buffer *buf, muduo::Timestamp time_)
	{
		muduo::string buf_(buf->retrieveAllAsString());
		LOG_INFO << conn->peerAddress().toIpPort() << " -> " \
			<< conn->localAddress().toIpPort() << " send " << buf_.size() << " data "
			<< " at time: " << time_.toFormattedString();
		conn->send(buf_);
	}
