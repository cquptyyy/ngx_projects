#ifndef MUDUO_BASE_ACCEPTOR_H
#define MUDUO_BASE_ACCEPTOR_H

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>


class EventLoop;

class Acceptor:noncopyable
{
public:
  using NewConnectionCallback=std::function<void(int sockfd,const InetAddress&)>;//处理获取到的新连接的回调函数类型

  Acceptor(EventLoop* loop,const InetAddress& addr,bool reuseport);
  ~Acceptor();
  void setNewConnectionCallback(const  NewConnectionCallback& cb){newConnectionCallback_=cb;}
  bool listenning()const{return listenning_;}
  void listen();
private:
  void handleRead();

  EventLoop* loop_;//接收用户定义的loop，也是mainloop/baseloop
  Socket acceptSocket_;//监听新连接套接字的对应的Socket对象
  Channel acceptChannel_;//监听套接字对应的Channel对象
  NewConnectionCallback newConnectionCallback_;//通过监听套接字获取新连接，处理新连接调用的回调函数
  bool listenning_;//记录是否处于监听状态
};
#endif