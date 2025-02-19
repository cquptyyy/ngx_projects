#ifndef MUDUO_BASE_SOCKET_H
#define MUDUO_BASE_SOCKET_H

#include "noncopyable.h"
#include "InetAddress.h"


//封装了用于监听的套接字sockfd
class Socket{
public:
  explicit Socket(int sockfd)
  :sockfd_(sockfd)
  {}

  ~Socket();
  int fd()const {return sockfd_;}
  void bindAddress(const InetAddress& localaddr);
  void listen();
  int accept(InetAddress* peeraddr);

  void shutdownWrite();

  void setTcpNoDelay(bool on);
  void setReuseAddr(bool on);
  void setReusePort(bool on);
  void setKeepAlive(bool on);
private:
  const int sockfd_;
};

#endif