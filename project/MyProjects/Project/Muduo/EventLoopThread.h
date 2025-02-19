#ifndef MUDUO_BASE_EVENTLOOPTHREAD_H
#define MUDUO_BASE_EVENTLOOPTHREAD_H

#include <mutex>
#include <functional>
#include <condition_variable>
#include "noncopyable.h"
#include "Thread.h"

/**
 * 封装一个thread和一个loop，每个thread在线程函数中启动一个loop
*/


class EventLoop;

class EventLoopThread:noncopyable{
public:
  using ThreadInitCallback=std::function<void(EventLoop*)>;

  EventLoopThread(const ThreadInitCallback& cb=ThreadInitCallback(),const std::string& name=std::string());
  ~EventLoopThread();
  //启动线程，
  EventLoop* startLoop();
private:
  void threadFunc();

  EventLoop* loop_;//事件循环
  bool exiting_;//标识事件循环结束，线程结束
  Thread thread_;//用于执行loop循环的thread
  std::mutex mutex_;//用于线程通信，返回loop时，loop必须已经创建成功
  std::condition_variable cond_;//用于线程通信
  ThreadInitCallback callback_;//初始化loop
};

#endif