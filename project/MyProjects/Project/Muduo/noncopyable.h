#ifndef MUDUO_BASE_NONCOPYABLE_H
#define MUDUO_BASE_NONCOPYABLE_H
/*
继承noncopyable的派生类不能拷贝构造和赋值，但可以析构和普通构造
*/
class noncopyable{
public:
  noncopyable(const noncopyable&)=delete;
  noncopyable& operator=(const noncopyable&)=delete;
protected:
  noncopyable()=default;
  ~noncopyable()=default;
};

#endif 