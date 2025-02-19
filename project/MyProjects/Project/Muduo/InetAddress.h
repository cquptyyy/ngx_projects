#ifndef MUDUO_BASE_INETADDRESS_H
#define MUDUO_BASE_INETADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

class InetAddress{
public:
  explicit InetAddress(uint16_t port=0,std::string ip="127.0.0.1");
  explicit InetAddress(const sockaddr_in& addr)
    :addr_(addr){}
  uint16_t toPort()const;
  std::string toIP()const;
  std::string toIpPort()const;
  const sockaddr_in* getSockAddr()const{return &addr_;}
  void setSockAddr(const sockaddr_in& addr){addr_=addr;}
private:
  sockaddr_in addr_;
};

#endif