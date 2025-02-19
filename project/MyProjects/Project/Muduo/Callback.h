#ifndef MUDUO_BASE_CALLBACK_H
#define MUDUO_BASE_CALLBACK_H

#include <memory>
#include <functional>

class Buffer;
class TcpConnection;
class TimeStamp;

using TcpConnectionPtr=std::shared_ptr<TcpConnection>;//tcp连接
using ConnectionCallback=std::function<void(const TcpConnectionPtr&)>;//新连接的回调函数
using CloseCallback=std::function<void(const TcpConnectionPtr&)>;//关闭连接的回调函数
using WriteCompleteCallback=std::function<void(const TcpConnectionPtr&)>;//写事件的回调函数
using MessageCallback=std::function<void(const TcpConnectionPtr&,Buffer*,TimeStamp)>;//读事件就绪的回调函数
using HighWaterMarkCallback=std::function<void(const TcpConnectionPtr&,size_t)>;


#endif