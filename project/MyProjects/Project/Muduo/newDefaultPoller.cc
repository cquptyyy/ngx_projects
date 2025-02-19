#include "Poller.h"
//#include "EpollPoller.h"

#include <stdlib.h>

//获取IO复用的对象
Poller* newDefaultPoller(EventLoop* loop){
  if(std::getenv("MUDUO_USE_POLL")){
    return nullptr;
  }
  else{
    return nullptr;
  }
}