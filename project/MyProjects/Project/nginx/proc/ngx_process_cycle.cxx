
#include <signal.h>
#include <errno.h>
#include "ngx_func.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_c_conf.h"

static void ngx_start_worker_process(int );
static void ngx_spawn_process(int,const char*);
static void ngx_worker_process_cycle(int,const char*);
static void ngx_process_init();

char master_process[]="master process";



//主进程循环
//先将信号屏蔽以免干扰创建子进程的过程
//设置进程标题
//创建子进程

void ngx_master_process_cycle(){
  sigset_t set;
  sigemptyset(&set);

  sigaddset(&set,SIGINT);
  sigaddset(&set,SIGHUP);
  sigaddset(&set,SIGQUIT);
  sigaddset(&set,SIGTERM);
  sigaddset(&set,SIGCHLD);
  sigaddset(&set,SIGIO);
  sigaddset(&set,SIGALRM);
  sigaddset(&set,SIGUSR1);
  sigaddset(&set,SIGUSR2);
  sigaddset(&set,SIGWINCH);

  if(sigprocmask(SIG_BLOCK,&set,nullptr)==-1){
    ngx_log_error_core(NGX_LOG_ALERT,errno,"ngx_master_process_cycle()中的sigprocmask() failed!");
  }
  
  size_t len=g_argvmemneed;
  len+=strlen(master_process)+1;
  if(len<1000){
    char title[1000]={0};
    strcpy(title,master_process);
    strcat(title,":");
    for(int i=0;i<g_os_argc;++i){
      strcat(title,g_os_argv[i]);
    }
    ngx_setproctitle(title);
  }

  CConfig* pconfig=CConfig::GetInstance();
  int workprocess=pconfig->GetIntDefault("workerprocesses",1);
  ngx_start_worker_process(workprocess);

  sigemptyset(&set);

  while(1){
    //sigsuspend将屏蔽信号集清空，允许接收所有信号
    //阻塞等待接收信号
    //收到信号恢复屏蔽信号集，执行信号处理函数
    ngx_log_error_core(0,0,"this is parent process pid=%d\n",ngx_pid);
    sigsuspend(&set);
  }

}

static void ngx_start_worker_process(int threadnum){
  for(int i=0;i<threadnum;++i){
    ngx_spawn_process(i,"worker process");
  }
}

static void ngx_spawn_process(int num,const char* pprocessname){
  pid_t pid=fork();
  if(pid==-1){
    ngx_log_error_core(NGX_LOG_EMERG,errno,
      "ngx_spawn_process()中的fork() failed threadnum=%d threadname=%s",num,pprocessname);
  }
  else if(pid==0){
    ngx_ppid=ngx_pid;
    ngx_pid=getpid();
    printf("creat a process pid=%d\n",ngx_pid);
    ngx_worker_process_cycle(num,pprocessname);
  }
}

static void ngx_worker_process_cycle(int num,const char* pprocessname){
  ngx_process_init();
  ngx_setproctitle(pprocessname);
  while(1){
    //ngx_log_error_core(0,0,"this child process threadnum=%d threadname=%s",num,pprocessname);
  }
}

static void ngx_process_init(){
  sigset_t set;
  sigemptyset(&set);
  if(sigprocmask(SIG_SETMASK,&set,nullptr)==-1){
    ngx_log_error_core(NGX_LOG_CRIT,errno,"ngx_process_init()中的sigprocmask() failed!");
  }
}