#ifndef PROTOCOLDISPATCHER_H
#define PROTOCOLDISPATCHER_H

#include <muduo/base/noncopyable.h>
#include <muduo/net/Callbacks.h>

#include <google/protobuf/message.h>

#include <map>

#include <type_traits>

using MessagePtr = std::shared_ptr<google::protobuf::Message>; // use smart pointer to manage Message

class Callback {
	public:
		virtual ~Callback() = default;
		virtual void onMessage(const muduo::net::TcpConnectionPtr &, const MessagePtr &, muduo::Timestamp) const = 0;
}; // why a base class is necessary? this is a important question

template <typename T>
class CallbackT: Callback {
	using MessageTCallbackFunc = std::function<void (const muduo::net::TcpConnectionPtr &, const std::shared_ptr<T> &, muduo::Timestamp)>;

	public:
	CallbackT (const MessageTCallbackFunc &func): callback_(func) {}

	void onMessage(const muduo::net::TcpConnectionPtr & conn, const MessagePtr & msg, muduo::Timestamp recvTime) const override {
		std::shared_ptr<T> corr_msg = muduo::down_pointer_cast<T>(msg); // dynamic_cast to corresponding message type, which is stored in T
		callback_(conn, corr_msg, recvTime);
	}

	private:
	MessageTCallbackFunc callback_;
};

class ProtocolDispatcher
// Dispatcher need: a default callback to handle unknown message
{
	using Callbacks = std::map<const google::protobuf::Descriptor *, std::shared_ptr<Callback>>;
	using ProtoBufMessageCallbackFunc = std::function<void (const muduo::net::TcpConnectionPtr &,
																													const MessagePtr &,
																													muduo::Timestamp)>;
private:
	Callbacks callbacks_;
	ProtoBufMessageCallbackFunc defaultCallback;
	

public:
	ProtocolDispatcher(const ProtoBufMessageCallbackFunc &func): defaultCallback(func) {}

	template <typename T>
		void register_callback(const typename CallbackT<T>::Callbacks &func){
			std::shared_ptr<CallbackT<T> > ptr_callback = new CallbackT<T> (func);
			callbacks_[T::Descriptor()] = ptr_callback;
		}

	void onProtobufMessage(const muduo::net::TcpConnectionPtr &conn, const MessagePtr &msg, muduo::Timestamp recvTime){
		auto itr = callbacks_.find(msg->GetDescriptor());
		if(itr != callbacks_.end()){
			itr->second->onMessage(conn, msg, recvTime);
		} else {
			defaultCallback(conn, msg, recvTime);
		}
	}

};

#endif /* PROTOCOLDISPATCHER_H */
