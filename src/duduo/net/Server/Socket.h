#ifndef SOCKET_H
#define SOCKET_H

#include "muduo/base/noncopyable.h"
#include <muduo/net/InetAddress.h>
#include <boost/noncopyable.hpp>

class Socket: boost::noncopyable
{
private:
  const int sockfd_;

public:
  int fd() const { return sockfd_; }
  void bindAddress(const muduo::net::InetAddress& localaddr);
  void listen();
  void shutdownWrite();

  // fill the peeraddr and return peer connection fd
  // If failed, return -1;
  int accept(muduo::net::InetAddress* peeraddr);

  // SO_REUSEADDR
  void setReuseAddr(bool on);

  // SO_REUSEPORT
  void setReusePort(bool on);
  explicit Socket(const int &fd);
  ~Socket();
};

#endif /* SOCKET_H */
