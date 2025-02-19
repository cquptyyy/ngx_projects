#ifndef MUDUO_BASE_EVENTLOOPTHREADPOOL_H
#define MUDUO_BASE_EVENTLOOPTHREADPOOL_H

#include "noncopyable.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool{
public:
  using ThreadInitCallback=std::function<void(EventLoop*)>;

  EventLoopThreadPool(EventLoop* baseloop,const std::string& nameArg);
  ~EventLoopThreadPool();

  void setThreadNum(int numThreads){numThreads_=numThreads;}
  void start(const ThreadInitCallback& cb=ThreadInitCallback());

  //获取下一个channel分配的loop
  EventLoop* getNextLoop();

  //获取所有线程执行的loop
  std::vector<EventLoop*> getAllLoops();
  bool started()const{return started_;}
  const std::string name()const{return name_;}
private:
  EventLoop* baseloop_;
  std::string name_;
  bool started_;//线程池是否启动
  int numThreads_;//线程池中线程的数量
  int next_;//下一派发channel的线程下标
  std::vector<std::unique_ptr<EventLoopThread>> threads_;//所有线程
  std::vector<EventLoop*> loops_;//所有线程执行的所有事件循环
};

#endif