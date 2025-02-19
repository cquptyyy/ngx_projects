#ifndef MUDUO_BASE_EVENTLOOP_H
#define MUDUO_BASE_EVENTLOOP_H

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include "noncopyable.h"
#include "TimeStamp.h"
#include "Poller.h"
#include "CurrentThread.h"

class Channel;
class Poller;

class EventLoop{
public:
  using Functor=std::function<void()>;

  EventLoop();
  ~EventLoop();
  
  //开启事件循环
  void loop();

  //停止事件循环
  void quit();

  TimeStamp pollReturnTime()const{return pollReturnTime_;};

  //在当前线程中执行cb
  void runInLoop(Functor cb);
  //将cb放入队列中，唤醒loop所在的线程，执行cb
  void queueInLoop(Functor cb);

  //唤醒loop所在的线程
  void wakeup();
  
  //Poller中的方法
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  void hasChannel(Channel* channel);

  //判断当前EventLoop对象是否在自己的线程中
  bool isInLoopThread(){return threadId_==CurrentThread::tid();}

private:
  void handleRead();//wakeup
  void doPendingFunctors();//执行回调

  using ChannelList=std::vector<Channel*>;

  //loop事件循环退出
  std::atomic_bool quit_;
  std::atomic_bool looping_;

  const pid_t threadId_;//当前线程的线程id

  TimeStamp pollReturnTime_;//IO复用对象返回事件
  std::unique_ptr<Poller> poller_;

  //监听套接字对应的文件描述符
  int wakeupFd_;//用于mainloop获取一个新用户连接，为连接找到一个subloop，通过wakeupFd唤醒subloop处理新连接
  std::unique_ptr<Channel> wakeupChannel_;//监听套接字对应文件描述符的channel

  ChannelList activeChannels_;

  std::atomic_bool callingPendingFunctors_;//标识当前线程是否有需要执行的回调方法
  std::vector<Functor> pendingFunctors_;//存储当前loop的回调函数
  std::mutex mutex_;//互斥锁，保证vector回调函数安全
};

#endif