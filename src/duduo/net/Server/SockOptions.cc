
#include "./SockOptions.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Types.h"
#include "muduo/net/Endian.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <sys/socket.h>
#include <sys/uio.h>  // readv
#include <unistd.h>

int sockoptions::createNonblockingOrDie(sa_family_t family){
  // stream == tcp
  int sockfd = ::socket(family,
                        SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 
                        IPPROTO_TCP);
  if(sockfd < 0){
    LOG_FATAL << "Create socket failed!";
  }
  return sockfd;
}

void sockoptions::bindOrDie(int sockfd, const struct sockaddr *addr){
  socklen_t len = static_cast<socklen_t>(sizeof(*addr));
  int ret = ::bind(sockfd, addr, len);
  if(ret < 0){
    LOG_FATAL << "Bind address failed!";
  }
}

void sockoptions::listenOrDie(int sockfd){
  int ret = ::listen(sockfd, SOMAXCONN);
  if(ret < 0){
    LOG_FATAL << "Listen socket failed!";
  }
}

void sockoptions::close(int sockfd){
  int ret = ::close(sockfd);
  if(ret < 0){
    LOG_FATAL << "Close sockfd falied!";
  }
}

int sockoptions::accept(int sockfd, struct sockaddr_in6 *addr){
  struct sockaddr *sa = sockaddr_cast(addr);
  socklen_t len = static_cast<socklen_t>(sizeof(*addr));
  int connfd = ::accept(sockfd, sa, &len);
  if(connfd < 0){
    LOG_FATAL << "Accept socket failed!";
  }
  return connfd;
}

const struct sockaddr * sockoptions::sockaddr_cast(const struct sockaddr_in *addr){
  return static_cast<const struct sockaddr *>(muduo::implicit_cast<const void*>(addr));
}

const struct sockaddr * sockoptions::sockaddr_cast(const struct sockaddr_in6 *addr){
  return static_cast<const struct sockaddr *>(muduo::implicit_cast<const void*>(addr));
}

struct sockaddr * sockoptions::sockaddr_cast(struct sockaddr_in6 *addr){
  return static_cast<struct sockaddr *>(muduo::implicit_cast<void *>(addr));
}
