#ifndef MUDUO_BASE_POLLER_H
#define MUDUO_BASE_POLLER_H

#include <vector>
#include <unordered_map>
#include "noncopyable.h"
#include "TimeStamp.h"

class Channel;
class EventLoop;

class Poller:noncopyable
{
public:
  using ChannelList=std::vector<Channel*>;

  Poller(EventLoop* loop);
  virtual ~Poller()=default;

  //启动IO复用监视文件描述符的就绪状态
  virtual TimeStamp poll(int timeoutMs,ChannelList* activeChannels)=0;
  //更新IO复用监视的文件描述符
  virtual void updateChannel(Channel* channel)=0;
  //从IO复用监视的channels中删除对应的channel即不再监视channel中的文件描述符
  virtual void removeChannel(Channel* channel)=0;

  //判断的文件描述符是否被监视
  bool hasChannel(Channel* channel)const;

  //获取IO复用的对象
  static Poller* newDefaultPoller(EventLoop* loop);

protected:
//key对应为sockfd value为sockfd对应的channel
  using ChannelMap=std::unordered_map<int,Channel*>;
  ChannelMap channels_;

private:
  EventLoop* ownerLoop_;
};

#endif