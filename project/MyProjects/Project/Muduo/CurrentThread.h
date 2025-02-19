#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include <unistd.h>
#include <syscall.h>

namespace CurrentThread{
  extern __thread int t_cachedTid;
  void cachedTid();

  inline int tid(){
    if(__builtin_expect(t_cachedTid==0,0)){
      cachedTid();
    }
    return t_cachedTid;
  }
}

#endif