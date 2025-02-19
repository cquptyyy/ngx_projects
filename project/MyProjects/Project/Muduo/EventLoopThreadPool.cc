#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop,const std::string& nameArg)
  :baseloop_(baseloop)
  ,name_(nameArg)
  ,started_(false)
  ,numThreads_(0)
  ,next_(0)
  {}


EventLoopThreadPool::~EventLoopThreadPool(){}


//创建并开启所有的线程，并把所有线程执行事件循环返回到loops中
void EventLoopThreadPool::start(const ThreadInitCallback& cb){
  started_=true;
  for(int i=0;i<numThreads_;++i){
    char buffer[name_.size()+32];
    snprintf(buffer,sizeof buffer,"%s%d",name_.c_str(),i);
    EventLoopThread *t=new EventLoopThread(cb,buffer);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));  
    loops_.push_back(t->startLoop());
  }
  if(numThreads_==0&&cb){
    cb(baseloop_);
  }
}


//获取下一个循环事件
EventLoop* EventLoopThreadPool::getNextLoop(){
  EventLoop* loop=baseloop_;
  if(!loops_.empty()){
    loop=loops_[next_];
    ++next_;
    if(next_==loops_.size()){
      next_=0;
    }
  }
  return loop;
}


//获取所有的循环事件
std::vector<EventLoop*>   EventLoopThreadPool::getAllLoops(){
  if(loops_.empty()){
    return std::vector<EventLoop*> (1,baseloop_);
  }
  else{
    return loops_;
  }
}