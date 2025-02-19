#ifndef MUDUO_BASE_TCPCONNECTION_H
#define MUDUO_BASE_TCPCONNECTION_H


#include "noncopyable.h"
#include "InetAddress.h"
#include "Callback.h"
#include "Buffer.h"

#include <string>
#include <atomic>
#include <memory>

class EventLoop;
class Channel;
class Socket;

class TcpConnection:noncopyable,public std::enable_shared_from_this<TcpConnection>
{
public:
  TcpConnection(EventLoop* loop,const std::string&name,int sockfd,
  const InetAddress& localAddr,const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop()const{return loop_;}
  const std::string& name()const{return name_;}
  const InetAddress& localAddress()const{return localAddr_;}
  const InetAddress& peerAddress()const{return peerAddr_;}

  bool connected()const{return state_==kConnected;}

  //发送数据
  void send(const std::string& buf);

  //关闭连接
  void shutdown();

  //设置新连接的回调函数
  void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_=cb;}

  //设置连接的读写事件的回调函数
  void setMessageCallback(const MessageCallback& cb){messageCallback_=cb;}

  //设置连接写事件完成的回调函数
  void setWriteCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_=cb;}

  //
  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb){highWaterMarkCallback_=cb;}

  //设置连接关闭的回调函数
  void setCloseCallback(const CloseCallback& cb){closeCallback_=cb;}

  //连接建立
  void connectEstablished();

  //连接销毁
  void connectDestroyed();
private:
  enum StateE{kDisConnected,kConnecting,kConnected,kDisConnecting};

  void setState(StateE state){state_=state;}

  //连接事件对应的处理函数
  void handleRead(TimeStamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  
  void sendInLoop(const void* message,size_t len);
  void shutdownInLoop();

  EventLoop* loop_;//不是baseloop 而是subloop,连接套接字对应的connfd对应的loop
  const std::string name_;//连接名字
  std::atomic_int  state_;//连接状态
  bool reading_;//

  std::unique_ptr<Socket> socket_;//连接套接字对应的socket对象
  std::unique_ptr<Channel> channel_;//连接套接字对应的channel对象

  const InetAddress localAddr_;//本地地址
  const InetAddress peerAddr_;//远端地址

  ConnectionCallback connectionCallback_;//有新连接时的回调函数
  MessageCallback messageCallback_;//有连接的读写事件就绪时的回调函数
  WriteCompleteCallback writeCompleteCallback_;//消息发送完的回调函数
  HighWaterMarkCallback highWaterMarkCallback_;//和发送能力和接收能力有关
  CloseCallback closeCallback_;//关闭连接的回调函数
  size_t highWaterMark_;

  Buffer inputBuffer_;//接收缓冲区
  Buffer outputBuffer_;//发送缓冲区
};



#endif