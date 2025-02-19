#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include <atomic>
#include <thread>
#include <functional>
#include <string>
#include <memory>
#include "noncopyable.h"

class Thread:noncopyable
{
public:
  using ThreadFunc=std::function<void()>;

  explicit Thread(ThreadFunc,const std::string &name=std::string());
  ~Thread();
  
  void start();
  void join();
  
  bool started()const{return started_;};
  pid_t tid()const{return tid_;}
  const std::string& name()const{return name_;}
  
  static int numCreated(){return numCreated_;}
private:
  void setDefaultName();

  bool started_;
  bool joined_;
  std::shared_ptr<std::thread> thread_; 
  pid_t tid_;
  std::string name_;
  ThreadFunc func_;
  static std::atomic_int numCreated_;
};

#endif