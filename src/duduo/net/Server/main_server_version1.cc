// #include "TcpServer.h"
// #include "../Reactor/EventLoop.h"
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
using namespace muduo::net;

EventLoop *g_loop = nullptr;

void connectionFunc(const TcpConnectionPtr &conn){
  // printf("A new TCP connection %s comming from %s\n", 
  //     conn->name().c_str(), conn->peerAddr().toIpPort().c_str());
  conn->send("Connection has established\n");
  // conn->shutdown();
}

void messageFunc(const TcpConnectionPtr &conn, muduo::net::Buffer *buf, muduo::Timestamp receiveTime){
  // printf("messageFunc - A new message has come from %s, which has %lu bytes data, receive time is %s\n", 
  //     conn->name().c_str(), buf->readableBytes(), receiveTime.toFormattedString().c_str());
  std::string msg = buf->retrieveAllAsString();
  printf("Message - %s\n", msg.c_str());
  conn->send("server sends this message back - " + msg);
}

void quitFunc(){
  g_loop->quit();
}

int main(int argc, char *argv[])
{
  EventLoop loop;
  g_loop = &loop;
  muduo::net::InetAddress addr(45670);
  TcpServer server(&loop, addr, "XXX server");
  server.setConnectionCallback(&connectionFunc);
  server.setMessageCallback(&messageFunc);
  // loop.runAfter(30, quitFunc);
  server.start();
  loop.loop();
  return 0;
}
