#ifndef CHAT_CLIENT_MYSELF_H
#define CHAT_CLIENT_MYSELF_H

#include "LengthHeaderCodec.h"
#include <muduo/net/TcpClient.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoopThread.h>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

class ChatClientMyself
{
private:
	TcpClient client_;
	TcpConnectionPtr conn_;
	LengthHeaderCodec codec_;
	mutable MutexLock mutex_;
	

public:
	ChatClientMyself(EventLoop *loop, const InetAddress &addr):
		client_(loop, addr, "CHAT_CLIENT_MYSELF"),
		codec_(std::bind(&ChatClientMyself::onStringMessage, this, _1, _2, _3))
	{
		client_.setConnectionCallback(std::bind(&ChatClientMyself::onConnection, this, _1));
		client_.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		client_.enableRetry(); // FIXME: what is this about?
	}

	void connect(){
		client_.connect();
	}

	void disconnect(){ // FIXME: is disconnect necessary?
		client_.disconnect();
	}

	void onConnection(const TcpConnectionPtr &conn){
		LOG_INFO << "Cient Server Connection: " \
			<< conn->localAddress().toIpPort() << " -> " \
			<< conn->peerAddress().toIpPort() << " is " \
			<< (conn->connected() ? "UP" : "DOWN");
		if(conn->connected()){
			conn_ = conn;
		} else {
			conn_.reset();
		}
	}

	void onStringMessage(const TcpConnectionPtr &, const string &message, Timestamp){
		// std::cout is not thread-safe, so we use printf instead;
		printf("<<< %s\n", message.c_str()); // print accepted data;
	}

	// send data from STDIN input;
	void write(const string &message){
		MutexLockGuard lock_(mutex_); // must lock mutex to protect shared_ptr;
		if(conn_){
			codec_.send(conn_.get(), message);
		}
	}


};

#endif /* CHAT_CLIENT_MYSELF_H */

int main(int argc, char *argv[])
{
	LOG_INFO << "pid = " << getpid();
	if(argc > 2){
		EventLoopThread loopThread; // for net-IO
		uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		InetAddress addr(argv[1], port);
		ChatClientMyself client(loopThread.startLoop(), addr);
		client.connect();

		std::string input_line;
		while(std::getline(std::cin, input_line)){
			client.write(input_line);
		}
		client.disconnect();
		CurrentThread::sleepUsec(1000 * 1000);
	} else {
		printf("Usage: %s <host-IP> <port>\n", argv[0]);
	}
	return 0;
}
