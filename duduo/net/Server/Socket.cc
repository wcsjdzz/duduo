#include "Socket.h"
#include "muduo/base/Logging.h"
#include "muduo/net/InetAddress.h"
#include "SockOptions.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>  // snprintf

Socket::Socket(const int &fd):
  sockfd_(fd)
{

}

Socket::~Socket(){
  sockoptions::close(sockfd_);
}

void Socket::shutdownWrite(){
  if(::shutdown(sockfd_, SHUT_WR)){
    LOG_ERROR << "Socket::shutdownWrite() - WRONG in ::shutdown";
  }
}

void Socket::bindAddress(const muduo::net::InetAddress &localaddr){
  sockoptions::bindOrDie(sockfd_, localaddr.getSockAddr());
}

void Socket::listen(){
  sockoptions::listenOrDie(sockfd_);
}

int Socket::accept(muduo::net::InetAddress *peeraddr){
  sockaddr_in6 tmpAddr;
  bzero(&tmpAddr, sizeof tmpAddr);
  int connfd = sockoptions::accept(sockfd_, &tmpAddr);
  peeraddr->setSockAddrInet6(tmpAddr);
  return connfd;
}

void Socket::setReuseAddr(bool on){
  int reused = on;
  int len = static_cast<socklen_t>(sizeof reused);
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reused, len);
  if(ret < 0){
    LOG_ERROR << "Socket::setReusedAddr() - falied";
  }
}

void Socket::setReusePort(bool on){
  int reused = on;
  int len = static_cast<socklen_t>(sizeof reused);
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &reused, len);
  if(ret < 0){
    LOG_ERROR << "Socket::setReusePort() - falied";
  }
}

void Socket::setTcpNoDealy(bool on){
  socklen_t tmp = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &tmp, sizeof tmp);
}

void Socket::setKeepAlive(bool on){
  socklen_t tmp = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, SO_KEEPALIVE, &tmp, sizeof tmp);
}
