#ifndef MUDUO_BASE_BUFFER_H
#define MUDUO_BASE_BUFFER_H

//prepend 预谋 预先考虑
//initial 最初的
//peek 偷看 偷窥
//retrieve 恢复 找回
//ensure 保证，确保
#include <vector>
#include <string>
#include <algorithm>

class Buffer{
public:
  //缓冲区中预留不使用的大小
  static const size_t kCheapPrepend=8;
  //默认缓冲区的大小
  static const size_t kInitialSize=1024;

  //传入缓冲区的初始大小，初始化成员变量
  explicit Buffer(size_t initialSize=kInitialSize)
    :buffer_(kCheapPrepend+initialSize)
    ,readIndex_(kCheapPrepend)
    ,writeIndex_(kCheapPrepend)
    {}
  

  //获取缓冲区中可读取数据的长度
  size_t readableBytes()const{return writeIndex_-readIndex_;}

  //获取缓冲区中可写入的空间大小
  size_t writableBytes()const{return buffer_.size()-writeIndex_;}

  //返回缓冲区中读取数据的开始位置
  size_t prependableBytes()const{return readIndex_;}

  //获取缓冲区中读取数据的开始地址
  const char* peek()const{return begin()+readIndex_;}

  //读取了len个数据，恢复读写位置
  void retrieves(size_t len){
    if(len<readableBytes()){
      readIndex_+=len;
    }
    else{
      retrieveAll();
    }
  }

  //读取了缓冲区所有的数据，恢复读写下标
  void retrieveAll(){readIndex_=writeIndex_=kCheapPrepend;}

  //读取缓冲区的所有数据
  std::string retrieveAllAsString(){return retrieveAsString(readableBytes());}

  //读取缓冲区len长度的数据
  std::string retrieveAsString(size_t len){
    std::string result(peek(),len);
    retrieves(len);
    return result;
  }

  //保证缓冲区能写入len长度的数据
  void ensureWritableBytes(size_t len){
    if(writableBytes()<len){
      makeSpace(len);
    }
  }

  //将date[0,len]的数据拷贝到缓冲区
  void append(const char* data,size_t len){
    ensureWritableBytes(len);
    std::copy(data,data+len,beginWrite());
    writeIndex_+=len;
  }

  //获取缓冲区可以写入的起始地址
  char* beginWrite(){return begin()+writeIndex_;}

  //获取缓冲区可以写入的起始地址
  const char* beginWrite()const{return begin()+writeIndex_;}

  //将fd上的数据读取到缓冲区
  ssize_t readFd(int fd,int* saveError);

  //将缓冲区的数据写入到fd上
  ssize_t writeFd(int fd,int* saveError);
private:
  //返回开始缓冲区的开始位置的地址
  char* begin(){return &*buffer_.begin();}
  
  //返回开始缓冲区的开始位置的地址
  const char* begin()const{return &*buffer_.begin();}

  
  //保证缓冲区的可写入的空间至少为len
  void makeSpace (size_t len){
    //缓冲区没有能写入的空间
    if(writableBytes()+prependableBytes()-kCheapPrepend<len){
      buffer_.resize(writeIndex_,len);
    }
    else{
      size_t readable=readableBytes();
      std::copy(begin()+readIndex_,begin()+writeIndex_,begin()+kCheapPrepend);
      readIndex_=kCheapPrepend;
      writeIndex_=readIndex_+readable;
    }
  }


  std::vector<char> buffer_;//缓冲区
  size_t readIndex_;//开始读的位置
  size_t writeIndex_;//开始写的位置 
};

#endif