//visual 视觉的
//remain v 保持 剩余


#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

static EventLoop* CheckLoopNotNull(EventLoop* loop){
  if(loop==nullptr){
    LOG_FATAL("%s:%s:%d TcpServer mainLoop is null!",__FILE__,__FUNCTION__,__LINE__);
  }
  return loop;
}


//构造函数，传入TcpConnection分配给的subLoop,nameArg?封装的fd,本地地址，远端地址
TcpConnection::TcpConnection(EventLoop* loop,const std::string& nameArg,
int sockfd,const InetAddress& localAddr,const InetAddress& peerAddr)
  :loop_(CheckLoopNotNull(loop))
  ,name_(nameArg)//连接的名字
  ,state_(kConnecting)//状态默认为正在连接
  ,reading_(true)
  ,socket_(new Socket(sockfd))  
  ,channel_(new Channel(loop,sockfd))
  ,localAddr_(localAddr)
  ,peerAddr_(peerAddr)
  ,highWaterMark_(64*1024*1024)
  {
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));

    LOG_DEBUG("TcpConnection::ctor[%s] at fd=%d",name.c_str(),sockfd);
    socket_->setKeepAlive(true);

  }



TcpConnection::~TcpConnection(){
  LOG_DEBUG("TcpConnection::dtor[%s] at fd=%d",name.c_str(),sockfd);
}


//文件描述符读事件就绪对应的处理函数
void TcpConnection::handleRead(TimeStamp receiveTime){
  int saveError=0;
  ssize_t n=inputBuffer_.readFd(channel_->fd(),&saveError);
  
  //若大于0 调用处理消息的回调函数
  if(n>0){messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);}
  else if(n==0){handleClose();}
  else{
    errno=saveError;
    handleError();
  }
}


//处理文件描述符的写事件
void TcpConnection::handleWrite(){
  //若文件描述符的写事件被epoll监听
  if(channel_->isWriting()){
    int saveError=0;
    ssize_t n=outputBuffer_.writeFd(channel_->fd(),&saveError);
    if(n>0){
      outputBuffer_.retrieves(n);
      //若已经将缓冲区的数据写完，不让epoll监听文件描述符的写资源就绪的状态
      if(outputBuffer_.readableBytes()==0){
        channel_->disableWriting();
        if(writeCompleteCallback_){
          loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
        }
        if(state_==kDisConnecting){
          shutdownInLoop();
        }
      }

    }
    else{
      LOG_ERROR("%s:%s:%d  write data from fd=%d error! errno=%d",__FILE__,__FUNCTION__,__LINE__,channel_->fd(),saveError);
    }
  }
  else{
    LOG_ERROR("TcpConnection fd=%d is down, no more writing!");
  }
}


//处理连接关闭
void TcpConnection::handleClose(){
  LOG_DEBUG("%s:%s:%d fd=%d state=%d",channel_->fd(),(int)state_);
  setState(kDisConnected);
  channel_->disableAll();

  //连接关闭前为啥还要connectionCallback_
  TcpConnectionPtr connPtr(shared_from_this());
  connectionCallback_(connPtr);
  closeCallback_(connPtr);
}


//处理连接错误
void TcpConnection::handleError(){
  int optVal;
  socklen_t optlen=sizeof(optVal);
  int err=0;
  if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optVal,&optlen)<0)err=errno;
  else err=optVal;
  LOG_ERROR("%s:%s:%d name:%s - SO_ERROR:%d",__FILE__,__FUNCTION__,__LINE__,name_.c_str(),err);
}


//发送数据
void TcpConnection::send(const std::string& buf){
  //如果已经建立了连接才发送数据
  if(kConnected==state_){
    //当前线程是连接所属的loop的线程
    if(loop_->isInLoopThread()){
      sendInLoop(buf.c_str(),buf.size());
    }
    else{
      loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,this,buf.c_str(),buf.size()));
    }
  }
}


//发送数据
void TcpConnection::sendInLoop(const void* data,size_t len){
  ssize_t nwrote=0;
  size_t remaining=len;

  bool faultError=false;//记录管道是否关闭或者双方连接状态不一致，连接是否被重置
  
  //如果连接已经关闭,就不能再向连接中写入数据
  if(state_==kDisConnected){
    LOG_ERROR("disconncted, give up writing!");
    return ;
  }

  //channel_->isWriting()==true表示已经要求epoll监听了连接的文件描述符是写事件的资源就绪
  //channel_->isWriting()==false表示没有要求epoll监听连接的写事件的资源就绪
  //若连接的写事件没有被epoll监听且输出缓冲区没有数据
  if(!channel_->isWriting()&&outputBuffer_.readableBytes()==0){
    nwrote=::write(channel_->fd(),data,len);
    if(nwrote>=0){
      remaining=len-nwrote;
      //若数据全部写到连接中
      if(remaining==0&&writeCompleteCallback_){
        loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
      }
    }
    //向文件描述符写数据失败了
    //EAGAIN||EWOULDBLOCK表示没有数据可读或者没有空间可写
    else{
      nwrote=0;
      if(errno!=EWOULDBLOCK){
        LOG_ERROR("%s:%s:%d write errno=%d",__FILE__,__FUNCTION__,__LINE__,errno);
        //errno==ECONNRESET 双发连接状态不一致，我方认为连接正常，对方认为连接已经关闭，
        //可能对方主机出现故障或者断电重新启动
        //errno==EPIPE连接所代表的管道已经关闭
        if(errno==EPIPE||errno==ECONNRESET){
          faultError=true;
        }
      }
    }
  }

  //若数据没有写完到连接中
  if(faultError==false&&remaining>0){
    size_t oldLen=outputBuffer_.readableBytes();
    if(oldLen+remaining>highWaterMark_&&oldLen<highWaterMark_&&highWaterMarkCallback_){
      loop_->queueInLoop(std::bind(highWaterMarkCallback_,shared_from_this(),oldLen+remaining));
    }
    outputBuffer_.append((char*)data+nwrote,remaining);
    if(channel_->isWriting()==false){channel_->enableWriting();}
  }
}


//连接建立
void TcpConnection::connectEstablished(){
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();

  //新连接建立，执行回调
  connectionCallback(shared_from_this());
}


//连接销毁
void TcpConnection::connectDestroyed(){
  if(state_==kConnected){
    setState(kDisConnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

//关闭连接
void TcpConnection::shutdown(){
  if(state_==kConnected){
    setState(kDisConnected);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
  }
}

//关闭连接
void TcpConnection::shutdownInLoop(){
  //输出缓冲区没有数据  关闭写端
  if(!channel_->isWriting()){socket_->shutdownWrite();}
}