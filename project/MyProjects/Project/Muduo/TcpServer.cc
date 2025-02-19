#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"

#include <strings.h>

TcpServer::TcpServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg,Option option)
  :loop_(loop)
  ,ipPort_(listenAddr.toIpPort())
  ,name_(nameArg)
  ,acceptor_(new Acceptor(loop,listenAddr,option==kReusePort))
  ,threadPool_(new EventLoopThreadPool(loop,name_))
  ,connectionCallback_()
  ,messageCallback_()
  ,nextConnId_(1)
  ,started_(0)
{
  acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,
  std::placeholders::_1,std::placeholders::_2));
}


//设置线程池的线程个数
void TcpServer::setThreadNum(int numThreads){
  threadPool_->setThreadNum(numThreads);
}


void TcpServer::start(){
  if(started_++==0){//防止一个tcpServer被多次start
      threadPool_->start(threadInitCallback_);//创建并启动多个线程
      loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
      //在主线程线程中运行mainloop，mainloop用来处理监听套接字的读事件
  }
}


//
void TcpServer::newConnection(int sockfd,const InetAddress& peerAddr){
  EventLoop*  ioLoop=threadPool_->getNextLoop();
  char buffer[64];
  snprintf(buffer,sizeof(buffer),"-%s#%d",ipPort_.c_str(),nextConnId_);
  ++nextConnId_;
  std::string connName=name_+buffer;
  LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s\n",
  name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

  sockaddr_in local;
  socklen_t len=sizeof local;
  ::bzero(&local,len);
  if(::getsockname(sockfd,(sockaddr*)&local,&len)<0){
    LOG_ERROR("TcpServer::getLocalAddr");
  }
  InetAddress localAddr(local);
  
  TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,sockfd,localAddr,peerAddr));
  connections_[connName]=conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);


  conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));

}


void TcpServer::removeConnection(const TcpConnectionPtr& conn){
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}


void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn){
  LOG_INFO("TcpServer::removeConnectionInLoop[%s] - connection %s\n",
  name_.c_str(),conn->name().c_str());
  
  connections_.erase(conn->name());
  EventLoop* ioLoop=conn->getLoop();
  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
}