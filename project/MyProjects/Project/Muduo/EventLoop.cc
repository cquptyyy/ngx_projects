#include "EventLoop.h"
#include "Logger.h"
#include "Channel.h"
#include <sys/eventfd.h>
#include <errno.h>
#include <unistd.h>


//防止一个线程创建多个EventLoop
__thread EventLoop* t_loopInThisThread=nullptr;

//IO复用的超时时间
const int kPollTimeMs=10000;


//创建wakeupfd,用来notify唤醒subloop来处理channel
int createEventfd(){
  int evtfd=::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
  if(evtfd<0){LOG_FATAL("func:%s eventfd error:%d",__FUNCTION__,errno);}
  return evtfd;
}


//构造函数，eventloop对应的线程 poller 用于唤醒每个eventloop事件循环的所需的wakefd
EventLoop::EventLoop()
  :looping_(false)
  ,quit_(false)
  ,callingPendingFunctors_(false)
  ,threadId_(CurrentThread::tid())
  ,poller_(Poller::newDefaultPoller(this))
  ,wakeupFd_(createEventfd())
  ,wakeupChannel_(new Channel(this,wakeupFd_))
  {
    LOG_DEBUG("EventLoop create %p in thread %d ",this,threadId_);
    if(t_loopInThisThread!=nullptr){
      LOG_FATAL("Another EventLoop %p exists in  this thread %d",t_loopInThisThread,threadId_);
    }
    else{
      t_loopInThisThread=this;
    }

    wakeupChannel_->setReadCallback(std::bind(handleRead,this));
    wakeupChannel_->enableReading();
  }


EventLoop::~EventLoop(){
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread=nullptr;
}


//开启事件循环，每次被唤醒都会执行事件处理方法和eventloop循环中的回调方法
void EventLoop::loop(){
  looping_=true;
  quit_=false;
  LOG_DEBUG("EventLoop %p is start looping",this);
  while(quit==false){
    activeChannels_.clear();
    pollReturnTime_=poller_->poll(kPollTimeMs,&activeChannels_);
    for(Channel* channel:activeChannels_){
      channel->handleEvent(pollReturnTime_);
    }
    doPendingFunctors();
  }
  LOG_DEBUG("EventLoop %p is stop looping",this);
  looping_=false;
}


//停止事件循环
void EventLoop::quit(){
  quit_=true;
  if(!isInLoopThread()){
    wakeup();
  }
}


//当前线程执行cb
void EventLoop::runInLoop(Functor cb){
  if(isInLoopThread()){
    cb();
  }
  else{
    queueInLoop(cb);
  }
}


//当前loop不在当前线程中，执行回调函数,需要将loop唤醒，由loop对应的线程执行
void EventLoop::queueInLoop(Functor cb){
  {
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.emplace_back(cb);
  }
  if(!isInLoopThread()||callingPendingFunctors_==true){
    wakeup();
  }
}


//通过wakeupFd读数据
void EventLoop::handleRead(){
  uint64_t one=1;
  ssize_t n=::read(wakeupFd_,&one,sizeof one);
  if(n!=sizeof one){
    LOG_ERROR("func:%s read %d bytes instead of 8",__FUNCTION__,n);
  }
}


//通过向wakeupFd中写数据，通知wakeupFd_唤醒对应的EventLoop去执行事件循环需要的方法
void EventLoop::wakeup(){
  uint64_t one=1;
  ssize_t n=::write(wakeupFd_,&one,sizeof one);
  if(n!=sizeof one){
    LOG_ERROR("func:%s write %d bytes instead of 8",__FUNCTION__,n);
  }
}


void EventLoop::updateChannel(Channel* channel){poller_->updateChannel(channel);}

void EventLoop::removeChannel(Channel* channel){poller_->removeChannel(channel);}

void EventLoop::hasChannel(Channel* channel){poller_->hasChannel(channel);}


//每次唤醒EventLoop事件循环都会执行EventLoop中的回调函数
void EventLoop::doPendingFunctors(){
  std::vector<Functor> functions;
  callingPendingFunctors_=true;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    functions.swap(pendingFunctors_);
  }
  for(const Functor& functor:functions){
    functor();
  }
  callingPendingFunctors_=false;
}


