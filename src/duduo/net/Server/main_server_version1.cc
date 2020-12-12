#include "TcpServer.h"
#include "../Reactor/EventLoop.h"

void connectionFunc(const TcpConnectionPtr &conn){
  // printf("A new TCP connection %s comming from %s\n", 
  //     conn->name().c_str(), conn->peerAddr().toIpPort().c_str());
  conn->send("this is a prompt message coming from server, indicating that the TCP connection has been establised already");
}

void messageFunc(const TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp receiveTime){
  printf("messageFunc - A new message has come from %s, which has %lu bytes data, receive time is %s\n", 
      conn->name().c_str(), buf->readableBytes(), receiveTime.toFormattedString().c_str());
  std::string msg = buf->retrieveAllAsString();
  printf("Message - %s\n", msg.c_str());
  conn->send("server sends this message back - " + msg);
}

int main(int argc, char *argv[])
{
  EventLoop loop;
  muduo::net::InetAddress addr(45670);
  TcpServer server(&loop, addr);
  server.setConnectionCallback(&connectionFunc);
  server.setMessageCallback(&messageFunc);

  server.start();
  loop.loop();
  return 0;
}
