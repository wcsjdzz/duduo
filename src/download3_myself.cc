#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

using namespace muduo::net;
using FilePtr = std::shared_ptr<FILE>; // we use shared_ptr to manager our file pointer;
#define BUF_SIZE (64*1024)

const char *file_name = nullptr;

void onConnection(const TcpConnectionPtr &conn){
	LOG_INFO << "FILE-SEND-SERVER: " << conn->peerAddress().toIpPort() \
		<< " -> " << conn->localAddress().toIpPort() << " is " \
		<< (conn->connected() ? "UP" : "DOWN");

	if(conn->connected()){
		FILE *ptr = ::fopen(file_name, "rb");
		if(!ptr){
			LOG_INFO << "error occurs while opening file";
			conn->shutdown();
		}
		FilePtr fp(ptr, ::fclose); // self-defined deleter
		conn->setContext(fp);
		char buf_[BUF_SIZE];
		size_t nRead = ::fread(buf_, 1, sizeof buf_, ptr);
		conn->send(buf_, nRead);
	}

}

void onWriteComplete(const TcpConnectionPtr &conn){
	const FilePtr &ptr = boost::any_cast<const FilePtr &>(conn->getContext());
	char buf_[BUF_SIZE];
	size_t nRead = ::fread(buf_, 1, sizeof buf_, ptr.get());
	if(!nRead)
		conn->shutdown();
	else
		conn->send(buf_, nRead);
}

int main(int argc, char *argv[])
{
	if(argc > 1){
		file_name = argv[1];
		EventLoop loop_;
		TcpServer server_(&loop_, InetAddress(2333), "FILE-SEND-SERVER");
		server_.setConnectionCallback(std::bind(&onConnection, std::placeholders::_1));
		server_.setWriteCompleteCallback(std::bind(&onWriteComplete, std::placeholders::_1));
		server_.start();
		loop_.loop();
	} else {
		fprintf(stderr, "Usage: %s <filename>", argv[0]);
	}
	return 0;
}

