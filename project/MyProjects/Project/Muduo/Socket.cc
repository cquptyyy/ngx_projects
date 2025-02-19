#include "Socket.h"
#include "Logger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>

Socket::~Socket(){
  ::close(sockfd_);
}


//传入绑定的ip+port与套接字进行绑定
void Socket::bindAddress(const InetAddress& localaddr){
  if(0!=::bind(sockfd_,(const sockaddr*)localaddr.getSockAddr(),sizeof(sockaddr_in))){
    LOG_FATAL("bind sockfd %d fail",sockfd_);
  }
}


//开启监听
void Socket::listen(){
  if(0!=::listen(sockfd_,1024)){
    LOG_FATAL("listen sockfd %d fail",sockfd_);
  }
}


//获取新连接,传入输入输出型的远程连接
int Socket::accept(InetAddress* peeraddr){
  sockaddr_in addr;
  socklen_t len=sizeof(addr);
  bzero(&addr,len);
  int connfd=::accept4(sockfd_,(sockaddr*)&addr,&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
  if(connfd>=0){
    peeraddr->setSockAddr(addr);
  }
  return connfd;
}


//关闭连接的写事件
void Socket::shutdownWrite(){
  if(::shutdown(sockfd_,SHUT_WR)<0){
    LOG_ERROR("sockfd %d shutdown error",sockfd_);
  }
}


//设置减少延迟
void Socket::setTcpNoDelay(bool on){
  int optval=on?1:0;
  ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof optval);
}


//设置处于TIME_WAIT状态下的套接字能够立即复用
void Socket::setReuseAddr(bool on){
  int optval=on?1:0;
  ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof optval);
}


//设置一个端口bind多个套接字
void Socket::setReusePort(bool on){
  int optval=on?1:0;
  ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&optval,sizeof optval);
}


//设置连接保活
void Socket::setKeepAlive(bool on){
  int optval=on?1:0;
  ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof optval);
}