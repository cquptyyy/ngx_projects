#include <errno.h>
#include <cstring>
#include <unistd.h>

#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

//表示channel未被添加到epoll中,没在channels中
const int kNew=-1; 
//表示channel被添加到epoll中，在channels中
const int kAdded=1;
//表示channel已经从epoll中删除，没在channels中
const int kDeleted=2;

EpollPoller::EpollPoller(EventLoop* loop)
  :Poller(loop)
  ,epollfd_(epoll_create1(EPOLL_CLOEXEC))//表示fork一个进程后关闭进程中的epollfd
  ,events_(kInitEventListSize)
  {
    if(epollfd_<0){
      LOG_FATAL("epoll_create errno:%d ,strerror:%s",errno,strerror(errno));
    }
  }

EpollPoller::~EpollPoller(){
  close(epollfd_);
}


//epoll_wait监视channel并将就绪的channel返回
TimeStamp EpollPoller::poll(int timeoutMs,ChannelList* activeChannels){
  LOG_DEBUG("func:%s fd total count:%lu",__FUNCTION__,channels_.size());

  int numEvents=epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
  int saveErrno=errno;
  TimeStamp now(TimeStamp::now());

  if(numEvents>0){
    LOG_DEBUG("%d events have happened",numEvents);
    if(numEvents==events_.size()){
      events_.resize(events_.size()*2);
    }
    fillActiveChannels(numEvents,activeChannels);
  }
  else if(numEvents==0){
    LOG_DEBUG("%s have timeout",__FUNCTION__);
  }
  else{
    if(saveErrno!=EINTR){
      errno=saveErrno;
      LOG_ERROR("%s error",__FUNCTION__);
    }
  }
  return now;
}


//将就绪的channel填写进activeChannels
void EpollPoller::fillActiveChannels(int numEvents,ChannelList* activeChannels){
  for(int i=0;i<numEvents;++i){
    Channel* channel=static_cast<Channel*>(events_[i].data.ptr);
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}


//更新epoll监视的channel
void EpollPoller::updateChannel(Channel* channel)
{
  int index=channel->index();

  LOG_DEBUG("func:%s fd:%d index:%d events:%d ",__FUNCTION__,channel->fd(),channel->index(),channel->events());

  //channel没有被epoll监视
  if(index==kNew||index==kDeleted){
    if(index==kNew){
      int fd=channel->fd();
      channels_[fd]=channel;
    }

    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD,channel);
  }


  //channel已经被epoll监视了
  else{
    //channel中没有需要监视的事件
    if(channel->isNonEvent()){
      update(EPOLL_CTL_DEL,channel);
      channel->set_index(kDeleted);
    }
    
    //将channel的事件更新
    else{
      update(EPOLL_CTL_MOD,channel);
    }
  }
}


//从epoll中删除channel
void EpollPoller::removeChannel(Channel* channel){
  int fd=channel->fd();
  int index=channel->index();
  channels_.erase(fd);

  LOG_DEBUG("func:%s fd:%d  ",__FUNCTION__,fd);

  if(index==kAdded){
    update(EPOLL_CTL_DEL,channel);

  }
  channel->set_index(kNew);
}


//根据操作数 对epoll进行更新
void EpollPoller::update(int operation,Channel* channel){
  struct epoll_event event;
  memset(&event,0,sizeof event);

  int fd=channel->fd();

  event.events=channel->events();
  event.data.ptr=channel;
  event.data.fd=fd;
  if(epoll_ctl(epollfd_,operation,fd,&event)<0){
    if(operation==EPOLL_CTL_DEL){
      LOG_ERROR("epoll_ctl del error=%d strerror=%s",errno,strerror(errno));
    }
    else{
      LOG_FATAL("epoll_ctl mod/add error=%d strerror=%s",errno,strerror(errno));
    }
  }
}
