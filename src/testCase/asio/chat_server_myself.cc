#ifndef CHAT_SERVER_MYSELF_H
#define CHAT_SERVER_MYSELF_H

#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <set>
#include "LengthHeaderCodec.h"

using namespace std::placeholders;

class ChatServerMyself
{
private:
	using connSet = std::set<muduo::net::TcpConnectionPtr>;


	LengthHeaderCodec codec_;
	muduo::net::TcpServer server_;
	connSet connects_;
	

public:
	ChatServerMyself(muduo::net::EventLoop *loop, muduo::net::InetAddress addr):
		server_(loop, addr, "CHAT_SERVER_MYSELF"),
		codec_(std::bind(&ChatServerMyself::onStringMessage, this, _1, _2, _3))
		{
			server_.setConnectionCallback(std::bind(&ChatServerMyself::onConnection, this, _1));
			server_.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		}


	void start();

	void onConnection(const muduo::net::TcpConnectionPtr &conn);
	void onStringMessage(const muduo::net::TcpConnectionPtr &conn, const muduo::string &message, muduo::Timestamp time);

};

#endif /* CHAT_SERVER_MYSELF_H */

void ChatServerMyself::start(){
	server_.start();
}

void ChatServerMyself::onConnection(const muduo::net::TcpConnectionPtr &conn){
	LOG_INFO << "ASIO Server: " << conn->peerAddress().toIpPort() \
		<< " -> " << conn->localAddress().toIpPort() << " is " \
		<< (conn->connected() ? "UP" : "DOWN");

	if(conn->connected()){
		connects_.insert(conn);
	} else {
		connects_.erase(conn);
	}
}

void ChatServerMyself::onStringMessage(const muduo::net::TcpConnectionPtr &, const muduo::string &message, muduo::Timestamp ){
	for(connSet::iterator itr = connects_.begin(); itr != connects_.end(); ++itr){ // send data to all clients it connects;
		codec_.send(itr->get(), message);
	}
}

int main(int argc, char *argv[])
{
	LOG_INFO << "pid = " << getpid();
	if(argc > 1){
		muduo::net::EventLoop loop;
		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		muduo::net::InetAddress addr(port);
		ChatServerMyself server(&loop, addr);
		server.start();
		loop.loop();
	} else {
		printf("Usage: %s <port>\n", argv[0]);
	}
	return 0;
}
