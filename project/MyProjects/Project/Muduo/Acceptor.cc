#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>


//创建一个非阻塞的文件描述符
static int createNonBlockFd(){
  int sockfd=::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_TCP);
  if(sockfd<0){
    LOG_FATAL("%s:%s:%d listen socket create err=%d",__FILE__,__FUNCTION__,__LINE__,errno);
  }
  return sockfd;
}


//构造函数传入用户定义的loop/mainloop/baseloop/  bind所需的ip+port的封装对象  以及是否重复使用的端口 
Acceptor::Acceptor(EventLoop* loop,const InetAddress& listenAddr,bool reuseport)
  :loop_(loop)
  ,acceptSocket_(createNonBlockFd())
  ,acceptChannel_(loop,acceptSocket_.fd())
  ,listenning_(false)
  {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
  }


//析构函数 将监听套接字对应的处理函数删除掉 ，将监听套接字从poller中删除掉
Acceptor::~Acceptor(){
  acceptChannel_.disableAll();
  acceptChannel_.remove();
}


//开启监听套接字的监听功能
//将监听套接字的读事件注册到poller中
void Acceptor::listen(){
  listenning_=true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}


//处理监听套接字的读事件就绪,也就是有新连接到来如何处理,接收新连接，调用处理新连接的回调函数
void  Acceptor::handleRead(){
  InetAddress peerAddr;
  int connfd=acceptSocket_.accept(&peerAddr);
  if(connfd>=0){
    if(newConnectionCallback_){
      newConnectionCallback_(connfd,peerAddr);
    }
    else{
      ::close(connfd);
    }
  }
  else{
    LOG_ERROR("%s:%s:%d accept err=%d",__FILE__,__FUNCTION__,__LINE__,errno);
    if(errno==EMFILE){
      LOG_ERROR("%s:%s:%d sockfd have reached limit!",__FILE__,__FUNCTION__,__LINE__);
    }
  }
}