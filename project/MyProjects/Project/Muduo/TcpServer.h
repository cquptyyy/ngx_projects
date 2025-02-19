#ifndef MUDUO_BASE_TCPSERVER_H
#define MUDUO_BASE_TCPSERVER_H

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callback.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

#include <string>
#include <atomic>
#include <memory>
#include <unordered_map>


class TcpServer:noncopyable{
public:
  using ThreadInitCallback=std::function<void(EventLoop*)>;

  enum Option{
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg, Option option=kNoReusePort);
  ~TcpServer();
  void setThreadInitCallback(const ThreadInitCallback& cb){threadInitCallback_=cb;}
  void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_=cb;}
  void setMessageCallback(const MessageCallback& cb){messageCallback_=cb;}
  void setWriteComplateCallback(const WriteCompleteCallback& cb){writeCompleteCallback_=cb;}

  void setThreadNum(int numThreads);

  void start();
private:
  void newConnection(int sockfd,const InetAddress& peerAddr);
  void removeConnection(const TcpConnectionPtr& conn);
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  using ConnectionMap =std::unordered_map<std::string,TcpConnectionPtr>;//连接的

  EventLoop* loop_;//用户传进来的mainloop
  
  const std::string name_;//tcpServer的名字
  const std::string ipPort_;//监听套接字的ip+port

  std::unique_ptr<Acceptor> acceptor_;//运行在mainloop,主要任务就是监听新连接到来的事件

  std::unique_ptr<EventLoopThreadPool> threadPool_;//线程池

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;

  ThreadInitCallback threadInitCallback_;//初始化loop

  std::atomic_int started_;

  int nextConnId_;
  ConnectionMap connections_;
};



#endif