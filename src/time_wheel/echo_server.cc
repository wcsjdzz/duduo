#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

#include <muduo/base/Logging.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <unordered_set>
#include <boost/circular_buffer.hpp>

using namespace muduo::net;
using namespace muduo;
using namespace std::placeholders;

class EchoServer
{
	using WeakTcpConnectionPtr = std::weak_ptr<TcpConnection>;
	struct Entry {
		std::weak_ptr<TcpConnection> ptr_;
		Entry(const WeakTcpConnectionPtr &ptr): ptr_(ptr) {}
		~Entry(){
			const TcpConnectionPtr conn = ptr_.lock(); // return the corresponding shared_ptr
			if(conn){ // must check the activity before shutdown action
				conn->shutdown();
			}
		}
	};

	using EntryPtr = std::shared_ptr<Entry>;
	using Bucket = std::unordered_set<EntryPtr>;
	using BucketList = boost::circular_buffer<Bucket>; // our finale data structure

	using WeakEntryPtr = std::weak_ptr<Entry>;

public:
	EchoServer(EventLoop *loop, const InetAddress &addr, const int &num_buckets):
		loop_(loop), server_(loop, addr, "ECHO SERVER"), buckets_(num_buckets)
	{
		server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1)); // must have the &
		server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
		loop_->runEvery(1.0, std::bind(&EchoServer::onTime, this));
	}
	// ~EchoServer();

	void start_(){
		server_.start();
	}

private:
	EventLoop *loop_;
	TcpServer server_;
	BucketList buckets_;

	void onTime(){
		buckets_.push_back(Bucket()); // the head bucket is popped automatically
	}

	void onConnection(const TcpConnectionPtr &conn){
		LOG_INFO << "ECHO_SERVER: " << conn->peerAddress().toIpPort()
			<< "-> " << conn->localAddress().toIpPort() << " is "
			<< ( conn->connected() ? " ON " : " OFF " );
		if(conn->connected()){
			EntryPtr newPtr(new Entry(conn)); // shared_ptr can be transformed to weak_ptr
			buckets_.back().insert(newPtr);
			conn->setContext(WeakEntryPtr(newPtr)); // store weak_ptr of Entry
		}
	}
	void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time_){
		muduo::string msg(buf->retrieveAllAsString());
		LOG_INFO << "ECHO_SERVER: " << conn->peerAddress().toIpPort() 
			<< " has message coming, size is: " << msg.size();
		WeakEntryPtr tmp = boost::any_cast<WeakEntryPtr>(conn->getContext());
		buckets_.back().insert(tmp.lock());// add the count
		conn->send(msg);
	}

	

};

#endif /* ECHO_SERVER_H */

int main(int argc, char *argv[])
{
	EventLoop loop;
	InetAddress addr(2333);
	EchoServer server(&loop, addr, 10);
	server.start_();
	loop.loop();
	return 0;
}
