#include <iostream>
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EpollPoller.h"

int main(){
  //std::cout<<TimeStamp::now().toString()<<std::endl;
  //std::cout<<"hell0"<<std::endl;
  //InetAddress inetaddr(8080,"43.136.89.242");
  //std::cout<<inetaddr.toIpPort()<<std::endl;
  EventLoop le;
  EventLoopThreadPool pool(&le,"thread");
  return 0;
}