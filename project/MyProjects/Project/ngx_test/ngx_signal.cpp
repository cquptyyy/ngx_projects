

#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "ngx_macro.h"
#include "ngx_func.h"

typedef struct{
  int signo;
  const char* signame;
  void (*handler)(int signo,siginfo_t* info,void* ucontext);
}ngx_signal_t; 


static void ngx_signal_handler(int signo,siginfo_t* info,void* ucontext);

ngx_signal_t signals[]={
  {SIGINT,"SIGINT",ngx_signal_handler},
  {SIGHUP,"SIGHUP",ngx_signal_handler},
  {SIGQUIT,"SIGQUIT",ngx_signal_handler},
  {SIGTERM,"SIGTERM",ngx_signal_handler},
  {SIGCHLD,"SIGCHLD",ngx_signal_handler},
  {SIGIO,"SIGIO",ngx_signal_handler},
  {SIGALRM,"SIGALRM",ngx_signal_handler},
  {SIGUSR1,"SIGUSER1",ngx_signal_handler},
  {SIGUSR2,"SIGUSER2",ngx_signal_handler},
  {SIGWINCH,"SIGWINCH",ngx_signal_handler},
  {SIGSYS,"SIGSYS",ngx_signal_handler},
  {0,nullptr,nullptr},
};

int ngx_signal_init(){
  ngx_signal_t* sig;
  struct sigaction sa;
  memset(&sa,0,sizeof(sa));

  for(sig=signals;sig->signo!=0;sig++){
    if(sig->handler!=nullptr){
      sa.sa_sigaction=sig->handler;
      sa.sa_flags=SA_SIGINFO;
    }
    else{sa.sa_handler=SIG_IGN;}

    sigemptyset(&sa.sa_mask);

    if(sigaction(sig->signo,&sa,nullptr)==-1)
    {ngx_log_error_core(NGX_LOG_EMERG,errno,"ngx_signal_init()中的sigaction(%s) failed",sig->signame);}
    else{ngx_log_stderr(0,"sigaction(%s) success! ",sig->signame);}
  }
  return 0;
}

static void ngx_signal_handler(int signo,siginfo_t* info,void* ucontext){
  printf("信号%d来了\n",signo);
}