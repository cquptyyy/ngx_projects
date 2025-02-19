#ifndef MODUO_BASE_EPOLLPOLLER_H
#define MODUO_BASE_EPOLLPOLLER_H

#include "Poller.h"
#include <vector>
#include <sys/epoll.h>

class EventLoop;

class EpollPoller:public Poller{
public:
  EpollPoller(EventLoop* loop);
  ~EpollPoller()override;

  //重写基类的虚函数
  //启动epoll 监视文件描述符的就绪状态
  TimeStamp poll(int timeoutMs,ChannelList* activeChannels)override;
  //更新epoll中监视的channel
  void updateChannel(Channel* channel)override;
  //从epoll监视的channels中删除channel
  void removeChannel(Channel* channel)override;

private:
//填写活跃的Channels,即将就绪的channel填进activeChannels中
  void fillActiveChannels(int numEvents,ChannelList* activeChannels);

//根据操作数更新Channel
  void update(int operation,Channel* channel);

//events_的初始大小
  static const int kInitEventListSize=16;
  using EventList=std::vector<epoll_event>;

  EventList events_;
  int epollfd_;
};

#endif