#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>

#define BUF_SIZE 1024*1024

const char *file_name = nullptr;

muduo::string reaf_file(){
	muduo::string content;
	FILE *fp = ::fopen(file_name, "rb");
	if(fp){
		char ioBuf[BUF_SIZE];
		::setbuffer(fp, ioBuf, sizeof ioBuf); // set io buffere for file stream;
		char buf_[BUF_SIZE];
		size_t read_sz = 0;
		while((read_sz = ::fread(buf_, 1, sizeof buf_, fp)) > 0){
			content.append(buf_, read_sz);
		}
	}
	return content;
}

void onConnect(const muduo::net::TcpConnectionPtr &conn){
	LOG_INFO << "Tcp connection " << conn->peerAddress().toIpPort() << " -> " \
		<< conn->localAddress().toIpPort() << " is " << (conn->connected() ? "Up" : "Down");

	if(conn->connected()){
			muduo::string content = reaf_file();
			conn->send(content); // send all data one time;
			conn->shutdown(); // close the tcp connection after data sent;
			LOG_INFO << "send over";
	} else {
		LOG_INFO << "Error occurred while opening file";
	}
}

int main(int argc, char *argv[])
{
	if(argc <= 1){
		LOG_INFO << "Usage: download_server <file_name>" ;
	} else {
		file_name = argv[1];
		muduo::net::EventLoop loop;
		muduo::net::InetAddress addr(2333);
		muduo::net::TcpServer server_(&loop, addr, "Download Server");
		server_.setConnectionCallback(bind(&onConnect, std::placeholders::_1));
		server_.start();
		loop.loop();
	}
	return 0;
}
