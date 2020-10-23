#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
using namespace muduo::net;

#define BUF_SIZE (64*1024)

const char *file_name; // get from argv;

void onConnect(const TcpConnectionPtr &conn){
	LOG_INFO << conn->peerAddress().toIpPort() << " -> " \
		<< conn->localAddress().toIpPort() << " is " \
		<< (conn->connected() ? "Up" : "Down") ;

	if(conn->connected()){
		FILE *ptr = ::fopen(file_name, "rb");
		if(ptr){
			conn->setContext(ptr);
			char buf_[BUF_SIZE];
			size_t nRead = ::fread(buf_, 1, sizeof buf_, ptr);
			conn->send(buf_, static_cast<int>(nRead));
		} else {
			LOG_INFO << "error occurred while opening file." ;
			conn->shutdown();
		}
	} else {
		if(!conn->getContext().empty()){
			FILE *ptr = boost::any_cast<FILE *>(conn->getContext());
			if(ptr){
				::fclose(ptr);
			}
		}
		LOG_INFO << "not connect" ;
	}
}

void onWriteComplete(const TcpConnectionPtr &conn){
	FILE *ptr = boost::any_cast<FILE *>(conn->getContext()); // there is difference bettween `boost` and `std`
	if(ptr){
		char buf_[BUF_SIZE];
		size_t nRead = ::fread(buf_, 1, sizeof buf_, ptr);
		if(nRead){
			conn->send(buf_, static_cast<int>(nRead));
		} else {
			::fclose(ptr);
			ptr = nullptr;
			conn->setContext(ptr);
			conn->shutdown(); // very important, client starts accepting data after `shutdown`
		}
	} else {
		LOG_INFO << "file send over." ;
		conn->shutdown();
	}
}

int main(int argc, char *argv[])
{
	if(argc == 1){
		LOG_INFO << "bad usage";
		exit(1);
	} 
	file_name = argv[1];
	EventLoop loop_;
	TcpServer server_(&loop_, InetAddress(2333), "File-Send-Server");
	server_.setConnectionCallback(std::bind(&onConnect, std::placeholders::_1));
	server_.setWriteCompleteCallback(std::bind(&onWriteComplete, std::placeholders::_1));
	server_.start();
	loop_.loop();
	return 0;
}

