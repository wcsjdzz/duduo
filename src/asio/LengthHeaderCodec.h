#ifndef LENGTHHEADERCODEC_H
#define LENGTHHEADERCODEC_H

#include <muduo/net/Buffer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>

class LengthHeaderCodec: muduo::noncopyable // this class is noncopyable
{
private:
	using MessageCallbackFunc = std::function<void (const muduo::net::TcpConnectionPtr &, const muduo::string &, muduo::Timestamp time)>;
	
	const int32_t nHeader = sizeof (int32_t); // length of header;
	MessageCallbackFunc messageCallback_; // this is a r-value;

public:
	explicit LengthHeaderCodec(const MessageCallbackFunc &func): messageCallback_(func)
	{}

	void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp time){
		// LOG_INFO << " buf->readableBytes() is " << buf->readableBytes(); // test, to show if data is accepted;
		// above LOG_INFO is OK, so bug comes below;
		while(buf->readableBytes() >= nHeader){ // could have data, but completeness can't be guaranteed
			// const void *dataptr = buf->peek();
			// int32_t dataSz = *static_cast<const int32_t *>(dataptr);
			// const int32_t len = muduo::net::sockets::networkToHost32(dataSz);
			const int32_t len = buf->peekInt32(); // this function contains networkToHost32
			if (len<0 || len>65536){
				LOG_ERROR << "Invalid length " << len;
				conn->shutdown();
				break;
			} else if(buf->readableBytes() < nHeader+len){
				break; // message is not complete;
			} else {
				buf->retrieve(nHeader);
				muduo::string message_(buf->peek(), len);
				// only @message_ is used here
				messageCallback_(conn, message_, time); // string message callback, registered by server and client
				buf->retrieve(len);
			}
		}
	}

	void send(muduo::net::TcpConnection *conn, const muduo::StringPiece &data){
		muduo::net::Buffer buf; 
		buf.append(data.data(), data.size()); // convert to type: muduo::net::Buffer
		int32_t len = static_cast<int32_t>(data.size());
		// int32_t dataSz = muduo::net::sockets::hostToNetwork32(len);
		// buf.prepend(&dataSz, sizeof dataSz);
		buf.prependInt32(len); // this function contains hostToNetwork32
		conn->send(&buf);
	}


};

#endif /* LENGTHHEADERCODEC_H */
