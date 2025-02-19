#include <semaphore.h>
#include "Thread.h"
#include "CurrentThread.h"

std::atomic_int Thread::numCreated_(0);


Thread::Thread(ThreadFunc func,const std::string& name)
  :started_(false)
  ,joined_(false)
  ,func_(std::move(func))
  ,tid_(0)
  ,name_(name)
  {
    setDefaultName();
  }


Thread::~Thread(){
  if(started_&&!joined_){
    thread_->detach();
  }
}


void Thread::start(){
  started_=true;
  sem_t sem;
  sem_init(&sem,false,0);
  thread_=std::shared_ptr<std::thread>(new std::thread([&]()->void{
    tid_=CurrentThread::tid();
    sem_post(&sem);
    func_();
  }));
  sem_wait(&sem);
}


void Thread::join(){
  joined_=true;
  thread_->join();
}


void Thread::setDefaultName(){
  int num=numCreated_;
  if(name_.empty()){
    char buffer[32]={0};
    snprintf(buffer,sizeof buffer,"Thread%d",num);
    name_=buffer;
  }
}











