#ifndef MUDUO_BASE_CHANNEL_H
#define MUDUO_BASE_CHANNEL_H

/** 
 * Channel封装了文件描述符fd，用户感兴趣的事件event，
 * poller返回的就绪的事件，以及处理不同事件的回调函数
*/


#include <functional>
#include <memory>
#include "noncopyable.h"
#include "TimeStamp.h"

class EventLoop; 

class Channel:noncopyable
{
public:
  using EventCallback=std::function<void()>;
  using ReadEventCallback=std::function<void(TimeStamp)>;

  Channel(EventLoop* loop,int fd);
  ~Channel();

  //fd有事件就绪poller通知channel处理就绪事件
  void handleEvent(TimeStamp receiveTime);

  //设置fd对应的事件处理函数
  void setReadCallback(ReadEventCallback cb){readCallback_=std::move(cb);}
  void setWriteCallback(EventCallback cb){writeCallback_=std::move(cb);}
  void setErrorCallback(EventCallback cb){errorCallback_=std::move(cb);}
  void setCloseCallback(EventCallback cb){closeCallback_=std::move(cb);}

  //防止channel已经被删除还在处理事件
  void tie(const std::shared_ptr<void>&);

  //返回fd关心的事件
  int events()const{return events_;}
  int fd()const{return fd_;}

  //设置fd已经就绪的事件
  void set_revents(int revents){revents_=revents;}

  //设置fd关心的事件
  void enableReading(){events_|=kReadEvent_;update();}
  void disableReading(){events_&=~kReadEvent_;update();}
  void enableWriting(){events_|=kWriteEvent_;update();}
  void disableWriting(){events_&=~kWriteEvent_;update();}
  void disableAll(){events_=kNonEvent_;update();}

  //返回fd对应的事件是否就绪
  bool isReading()const {return events_&kReadEvent_;}
  bool isWriting()const {return events_&kWriteEvent_;}
  bool isNonEvent()const{return events_==kNonEvent_;}

  int index()const{return index_;}
  void set_index(int index){index_=index;}

  EventLoop* ownerLoop()const{return loop_;}
  void remove();

private:
  //更新poller应该关心的事件
  void update();
  void handleEventWithGuard(TimeStamp receiveTime);


  const static int kNonEvent_;
  const static int kReadEvent_;
  const static int kWriteEvent_;

  EventLoop* loop_;
  const int fd_;
  int events_;//用户关心的事件 如EPOLLIN EPOLLOUT
  int revents_;//poller返回的就绪事件
  int index_;//用于记录channel在epoll中的状态

  std::weak_ptr<void> tie_;
  bool tied_;
  

  //事件对应的回调函数
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback errorCallback_;
  EventCallback closeCallback_;
};

#endif