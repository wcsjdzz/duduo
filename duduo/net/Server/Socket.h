#ifndef SOCKET_H
#define SOCKET_H

#include <muduo/net/InetAddress.h>

class Socket
{
private:
  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  const int sockfd_;

public:
  int fd() const { return sockfd_; }
  void bindAddress(const muduo::net::InetAddress& localaddr);
  void listen(); // `::listen()` peer connection request
  void shutdownWrite();

  // fill the peeraddr and return peer connection fd
  // If failed, return -1;
  int accept(muduo::net::InetAddress* peeraddr);

  // SO_REUSEADDR
  void setReuseAddr(bool on);

  // SO_REUSEPORT
  void setReusePort(bool on);

  void setTcpNoDealy(bool on);
  void setKeepAlive(bool on);
  explicit Socket(const int &fd);
  ~Socket();
};

#endif /* SOCKET_H */
