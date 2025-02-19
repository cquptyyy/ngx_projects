
#include <iostream>
#include <mutex>
#include <string.h>
#include "ngx_c_conf.h"
#include "ngx_func.h"
#include <unistd.h>
#include "ngx_global.h"
#include "ngx_macro.h"
using namespace std;

static void freeresource();


char** g_os_argv;//指向命令参数
char* gp_envmem;//指向保存环境变量的新内存
int g_environlen=0;//环境变量的长度
pid_t ngx_pid;//进程id
int g_envmemneed=0;//环境变量的长度
int g_argvmemneed=0;//参数的长度
int g_os_argc=0;//参数的个数
pid_t ngx_ppid;
int main(int argc, char *const *argv)
{   
	int exitcode = 0;           //退出代码，先给0表示正常退出

	//(1)无伤大雅也不需要释放的放最上边    
	ngx_pid = getpid();   
	ngx_ppid=getppid();      //取得进程pid
	g_os_argv = (char **) argv; //保存参数指针  
	g_os_argc=argc;//保存参数的个数  
	for(int i=0;i<argc;++i){g_argvmemneed+=strlen(argv[i])+1;}
	for(int i=0;environ[i];++i){g_envmemneed+=strlen(environ[i])+1;}

	//(2)初始化失败，就要直接退出的
	//配置文件必须最先要，后边初始化啥的都用，所以先把配置读出来，供后续使用 
	CConfig *p_config = CConfig::GetInstance(); //单例类
	if(p_config->Load("nginx.conf") == false) //把配置文件内容载入到内存        
	{        
			ngx_log_stderr(0,"配置文件[%s]载入失败，退出!","nginx.conf");
			//exit(1);终止进程，在main中出现和return效果一样 ,exit(0)表示程序正常, exit(1)/exit(-1)表示程序异常退出，exit(2)表示表示系统找不到指定的文件
			exitcode = 2; //标记找不到文件
			freeresource();  //一系列的main返回前的释放动作函数
			printf("程序退出，再见!\n");
			return exitcode;
	}
	
	//(3)一些初始化函数，准备放这里
	ngx_log_init();             //日志初始化(创建/打开日志文件)
	if(ngx_signal_init()){      //初始化信号的处理函数
		return 1;
	}


	//(4)一些不好归类的其他类别的代码，准备放这里
	ngx_init_setproctitle();    //把环境变量搬家

	//(开启主进程创建)
	ngx_master_process_cycle();

	ngx_log_error_core(5,8,"这个XXX工作的有问题,显示的结果是=%s","YYYY");
	//--------------------------------------------------------------    
	// for(;;)
	// {
	// 		sleep(1); //休息1秒        
	// 		printf("休息1秒\n");        

	// }
		

	freeresource();  //一系列的main返回前的释放动作函数
	printf("程序退出，再见!\n");
	return exitcode;

}
static void freeresource(){
	if(ngx_log.fd!=-1&&ngx_log.fd!=STDERR_FILENO){
		close(ngx_log.fd);
		ngx_log.fd=-1;
	}
	if(gp_envmem!=nullptr){
		delete[] gp_envmem;
		gp_envmem=nullptr;
	}
}
