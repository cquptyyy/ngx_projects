#include "Channel.h"

#include <sys/epoll.h>

const int Channel::kNonEvent_=0;
const int Channel::kReadEvent_=EPOLLIN|EPOLLPRI;
const int Channel::kWriteEvent_=EPOLLOUT;

Channel::Channel(EventLoop* loop,int fd)
  :loop_(loop),fd_(fd),events_(0),revents_(0),index_(-1),tied_(false)
  {}

Channel::~Channel(){}

//fd对应的事件已更新，需要从新注册进poller中
void Channel::update(){
  //loop_->updateChannel(this);
}


//将fd对应的channel删除
void Channel::remove(){
  //loop_->removeChannel(this);
}

//什么时候调用tie
void Channel::tie(const std::shared_ptr<void>& obj){
  tie_=obj;
  tied_=true;
}

//fd有事件就绪，调用事件处理函数
void Channel::handleEvent(TimeStamp receiveTime){
  if(tied_==true){
    std::shared_ptr<void> guard=tie_.lock();
    if(guard!=nullptr){
      handleEventWithGuard(receiveTime);
    }
  }
  else{
    handleEventWithGuard(receiveTime);
  }
}

//对不同事件调用不同的回调函数
void Channel::handleEventWithGuard(TimeStamp receiveTime){
  if((events_&EPOLLHUP)&!(events_&EPOLLIN)){
    if(closeCallback_!=nullptr){
      closeCallback_();
    }
  }
  if(events_&EPOLLERR){
    if(errorCallback_!=nullptr){
      errorCallback_();
    }
  }
  if(events_&EPOLLOUT){
    if(writeCallback_!=nullptr){
      writeCallback_();
    }
  }
  if(events_&(EPOLLIN|EPOLLPRI)){
    if(readCallback_!=nullptr){
      readCallback_(receiveTime);
    }
  }
}