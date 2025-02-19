
#include "InetAddress.h"
#include <strings.h>
#include <string.h>
InetAddress::InetAddress(uint16_t port,std::string ip){
  bzero(&addr_,sizeof addr_);
  addr_.sin_family=AF_INET;
  addr_.sin_addr.s_addr=inet_addr(ip.c_str());
  addr_.sin_port=htons(port);
}
uint16_t InetAddress::toPort()const{
  return ntohs(addr_.sin_port);
}
std::string InetAddress::toIP()const{
  char buffer[128];
  inet_ntop(AF_INET,&addr_.sin_addr,buffer,sizeof buffer);
  return buffer;
}
std::string InetAddress::toIpPort()const{
  char buffer[128];
  inet_ntop(AF_INET,&addr_.sin_addr,buffer,sizeof buffer);
  size_t end=strlen(buffer);
  uint16_t port=ntohs(addr_.sin_port);
  sprintf(buffer+end,":%u",port);
  return buffer;
}