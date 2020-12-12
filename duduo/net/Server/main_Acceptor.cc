#include "../Reactor/EventLoop.h"
#include "Acceptor.h"
#include <muduo/net/InetAddress.h>

void callbackFunc(int connfd, const muduo::net::InetAddress &addr){
  printf("A new connection comming from %s\n", addr.toIpPort().c_str());
  char msg[] = "Hello, I can hear you calling me\n";
  ::write(connfd, msg, sizeof msg);
}

int main(int argc, char *argv[])
{
  EventLoop loop;
  muduo::net::InetAddress localAddr(2333);
  muduo::net::InetAddress localAddr2(3332);
  Acceptor acceptor(&loop, localAddr);
  Acceptor acceptor2(&loop, localAddr2);
  acceptor.setNewConnectionCallback(&callbackFunc);
  acceptor2.setNewConnectionCallback(&callbackFunc);
  acceptor.listen();
  acceptor2.listen();
  loop.loop();
  return 0;
}
