#include "Buffer.h"

#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>


//将fd上的数据读取到缓冲区
ssize_t Buffer::readFd(int fd,int* saveError){
  char extrabuf[65536]={0};

  struct iovec vec[2];

  const int writable=writableBytes();

  vec[0].iov_base=begin()+writeIndex_;
  vec[0].iov_len=writable;

  vec[1].iov_base=extrabuf;
  vec[1].iov_len=sizeof extrabuf;

  const int iovcnt=(writable< sizeof(extrabuf))?2:1;
  ssize_t n=::readv(fd,vec,iovcnt);
  if(n<0){*saveError=errno;}
  else if(n<=writable)writeIndex_+=n;
  else{
    writeIndex_=buffer_.size();
    append(extrabuf,n-writable);
  }
  return n;
}

//将缓冲区的数据写入到fd上
ssize_t Buffer::writeFd(int fd,int* saveError){
  ssize_t n=::write(fd,peek(),readableBytes());
  if(n<0){
    *saveError=errno;
  }
  return n;
}