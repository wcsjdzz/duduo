#ifndef PROTOCOLDISPATCHER_H
#define PROTOCOLDISPATCHER_H

#include <muduo/base/noncopyable.h>
#include <muduo/net/Callbacks.h>

#include <google/protobuf/message.h>

#include <map>

#include <type_traits>

using ProtobufMessagePtr = std::shared_ptr<google::protobuf::Message>;

class Callback { // a virtual base class, for convience of using smart pointer
	public:
		virtual ~Callback() = default;
		virtual void onProtobufMessage(const muduo::net::TcpConnectionPtr &,
																		const ProtobufMessagePtr &,
																		muduo::Timestamp) const = 0;

};

template <typename T>
class CallbackT: Callback {
	using ProtobufMessageTCallback = std::function<void (const muduo::net::TcpConnectionPtr &,
																												const std::shared_ptr<T> &,
																												muduo::Timestamp )>;
	public:
		CallbackT(const ProtobufMessageTCallback &func): callback_(func) {}

		void onProtobufMessage(const muduo::net::TcpConnectionPtr & conn,
																		const ProtobufMessagePtr & msg,
																		muduo::Timestamp recvTime) const override {
			std::shared_ptr<T> corr_msg = muduo::down_pointer_cast<T>(msg); // Message type can be found in typename T
			callback_(conn, corr_msg, recvTime);
		}

	private:
		ProtobufMessageTCallback callback_;
};

class ProtocolDispatcher
{
	using ProtobufMessageCallback = std::function<void (const muduo::net::TcpConnectionPtr &, 
																											 const ProtobufMessagePtr &,
																											 muduo::Timestamp )>;
	using CallbacksMap = std::map<const google::protobuf::Descriptor *, std::shared_ptr<Callback>> ; // here, the virtual base class starts to come in handy;

private:
	ProtobufMessageCallback defaultCallback_;
	CallbacksMap callbacks_;
	

public:
	ProtocolDispatcher(const ProtobufMessageCallback &func): defaultCallback_(func) {}

	template <typename T>
		void register_protobuf_callback(const typename T::ProtobufMessageTCallback &func){
			std::shared_ptr<CallbackT<T>> item_(func); 
			callbacks_[T::Descriptor()] = item_;
		}

	void onProtobufMessageCallback(const muduo::net::TcpConnectionPtr &conn, 
																	const ProtobufMessagePtr &msg, 
																	muduo::Timestamp recvTime){
		const auto itr = callbacks_.find(msg->GetDescriptor());
		if(itr != callbacks_.cend()){
			itr->second->onProtobufMessage(conn, msg, recvTime);
		} else {
			defaultCallback_(conn, msg, recvTime);
		}
	}
};

#endif /* PROTOCOLDISPATCHER_H */
