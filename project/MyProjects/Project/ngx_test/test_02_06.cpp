
//std::mutex mtx;
//class CConfig{
//public:
//	static CConfig* getInstance() {
//		/*if (instance == nullptr) {
//			instance = new CConfig();
//		}*/
//		return &instance;
//	}
//private:
//	/*class CRelease {
//	public:
//		~CRelease() {
//			if (CConfig::instance != nullptr) {
//				delete CConfig::instance;
//				CConfig::instance = nullptr;
//			}
//		}
//	};*/
//	CConfig() { cout << "构造函数" << endl; }
//	CConfig(const CConfig&) = delete;
//	CConfig& operator=(const CConfig&) = delete;
//	~CConfig() { cout << "析构函数" << endl; }
//	static CConfig instance;
//	//static CRelease release;//静态对象程序结束自动调用析构函数
//};
////CConfig* CConfig::instance = nullptr;
////CConfig::CRelease CConfig::release;
//CConfig CConfig::instance;
//int main() {
//	cout << "---" << endl;
//	CConfig* pc = CConfig::getInstance();
//	CConfig* pc1 = CConfig::getInstance();
//	CConfig* pc2 = CConfig::getInstance();
//	CConfig* pc3 = CConfig::getInstance();
//	CConfig* pc4 = CConfig::getInstance();
//	return 0;
//}



////饿汉式单例模式
//class Singleton {
//public:
//	static Singleton* getInstance() {
//		return &instance;
//	}
//private:
//	Singleton() { cout << "构造函数"<<endl; }
//	~Singleton() { cout << "析构函数" << endl; }
//	Singleton(const Singleton&) = delete;
//	Singleton& operator=(const Singleton&) = delete;
//	static Singleton instance;
//};
//Singleton Singleton::instance;



//class Singleton {
//public:
//	static Singleton* getInstance() {
//		
//		if (instance == nullptr) {
//			//开辟内存 调用构造函数  赋值  不是线程安全的函数
//			lock_guard<std::mutex> guard(mtx);//锁的粒度过大   若重复调用已经存在单例对象  还继续加锁
//			//若多个线程进入这里会修改instance的值，导致会出现多个单例对象  就不是单例模式
//			//所以这里还需要再判断单例对象是否已经存在
//			if (instance == nullptr) {
//				instance = new Singleton();
//			}
//		}
//		return instance;
//	}
//private:
//	class CRelease {
//	public:
//		~CRelease() {
//			if (Singleton::instance != nullptr) {
//				delete Singleton::instance;
//				Singleton::instance = nullptr;
//			}
//		}
//	};
//	Singleton() { cout << "构造函数" << endl; }
//	~Singleton() { cout << "析构函数" << endl; }
//	Singleton(const Singleton&) = delete;
//	Singleton& operator=(const Singleton&) = delete;
//	static Singleton* volatile instance;
//	//静态变量属于数据段的数据 同一个进程的所有线程都会共享这个变量  
//	// 为了保证线程缓存共享的数据  实时知道共享数据的修改  使用volatile进行修饰
//	static CRelease release;//静态对象程序结束自动析构
//	static std::mutex mtx;
//	
//};
//Singleton*volatile Singleton::instance=nullptr;
//Singleton::CRelease Singleton::release;
//std::mutex Singleton::mtx;
//
//int main() {
//	//cout << "-------------" << endl;
//	Singleton* ps = Singleton::getInstance();
//	Singleton* ps1 = Singleton::getInstance();
//	Singleton* ps2 = Singleton::getInstance();
//	return 0;
//}



//#include <iostream>
//
//#include <stdio.h>
//
//
//void Rtrim(char* string) {
//	if (string==nullptr)return;
//	size_t len = strlen(string);
//	while (len > 0 && string[len - 1] == ' ') {
//		string[len-1] = '\0';
//		len--;
//	}
//}
//
//void Ltrim(char* string) {
//	if (string == nullptr)return;
//	size_t len = strlen(string);
//	char* p_tmp = string;
//	if (*p_tmp != ' ')return;
//	while (*p_tmp != '\0') {
//		if (*p_tmp == ' ')p_tmp++;
//		else break;
//	}
//	if (*p_tmp == '\0') {
//		*string='\0';
//		return;
//	}
//	char* p_tmp2 = string;
//	while (*p_tmp != '\0') {
//		*p_tmp2 = *p_tmp;
//		p_tmp2++;
//		p_tmp++;
//	}
//	*p_tmp2 = '\0';
//	return;
//}
//
//
//
//class CConfig {
//public:
//	static CConfig* getInstance() {
//		if (instance != nullptr) {
//			lock_guard<std::mutex> lock(mtx);
//			if (instance != nullptr) {
//				instance = new CConfig();
//			}
//		}
//		return instance;
//	}
//private:
//	class CRlease {
//	public:
//		~CRlease() {
//			if (CConfig::instance != nullptr) {
//				delete CConfig::instance;
//				CConfig::instance = nullptr;
//			}
//		}
//	};
//	CConfig(){}
//	~CConfig(){}
//	CConfig(const CConfig&) = delete;
//	CConfig& operator=(const CConfig&) = delete;
//	static CConfig* instance;
//	static std::mutex mtx;
//public:
//	void Load(const char* pconfigName);
//
//};
//std::mutex CConfig::mtx;
//CConfig* CConfig::instance = nullptr;




#include "ngx_func.h"
#include <iostream>
#include <mutex>
#include <string.h>
#include "ngx_c_conf.h"
#include "ngx_func.h"
#include <unistd.h>
#include "ngx_global.h"
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

int main(int argc,char* const* argv) {
	//g_os_argv=(char**)argv;
	// cout << "hell0" << endl;
	// char buffer[1024] = "    hello  =   world       ";
	// char* tmp=strchr(buffer, '=');
	// char s1[24];
	// char s2[24];
	// memset(s1, 0, sizeof(s1));
	// memset(s2, 0, sizeof(s2));
	// strncpy(s1, buffer, tmp - buffer);
	// strcpy(s2, tmp+1);
	// printf("s1=%s\ns2=%s\n", s1, s2);
	// Rtrim(s1);
	// Ltrim(s1);
	// Rtrim(s2);
	// Ltrim(s2);q
	//strcpy(argv[0],"ngx_test+++");
	// for(int i=0;argv[i];++i){
	// 	printf("argv[%d]=%s\n",i,argv[i]);
	// }
	// printf("argv[0]=%p,environ[0]=%p\n",&argv[0],environ);
	// CConfig* pconfig=CConfig::GetInstance();
	// if(pconfig->Load("nginx.conf")==false){
	// 	write(2,"loaderror",sizeof("loaderror"));
	// };
	// int port=pconfig->GetIntDefault("Port",-1);
	// string host=pconfig->GetString("HoSt");
	// cout<<port<<"   "<<host<<endl;
	// // while(1){
	// // 	printf("hello\n");
	// // 	sleep(1);
	// // }
	// for(int i=0;environ[i];++i){
	// 	printf("environ[%d]=%s\n",i,environ[i]);
	// }
	// printf("-------------------------------\n");
	// ngx_init_setproctitle();

	// for(int i=0;environ[i];++i){
	// 	printf("environ[%d]=%s\n",i,environ[i]);
	// }


	// ngx_setproctitle("nginx:master process");
	// while(1){
	// 	printf("hello\n");
	// 	sleep(1);
	// }
	//write(1,"hello",7);
	// ngx_log_stderr(0,"ngx_test %d %.2f",789,3.996);
	// ngx_log_stderr(0, "invalid option: \"%s\"", argv[0]);  //nginx: invalid option: "./nginx"
	// ngx_log_stderr(0, "invalid option: %10d", 21);         //nginx: invalid option:         21  ---21前面有8个空格
	// ngx_log_stderr(0, "invalid option: %.6f", 21.378);     //nginx: invalid option: 21.378000   ---%.这种只跟f配合有效，往末尾填充0
	// ngx_log_stderr(0, "invalid option: %.6f", 12.999);     //nginx: invalid option: 12.999000
	// ngx_log_stderr(0, "invalid option: %.2f", 12.999);     //nginx: invalid option: 13.00
	// ngx_log_stderr(0, "invalid option: %x", 1678);        //nginx: invalid option: 68E
	// ngx_log_stderr(0, "invalid option: %X", 1678);        //nginx: invalid option: 68E
	// ngx_log_stderr(15, "invalid option: %s , %d", "testInfo",326);        //nginx: invalid option: testInfo , 326
	// ngx_log_stderr(0, "invalid option: %d", 1678); 
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