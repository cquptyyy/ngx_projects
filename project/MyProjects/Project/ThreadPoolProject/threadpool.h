//#pragma once ��vs�µķ�ֹͷ�ļ��ظ���������linux�²�һ������
//linux��ʹ��ifndef  THREADPOOL_H #define THREADPOOL_H #endif  
//��ֹͷ�ļ��ظ�����
//SDK Software Development Kit ���������
//OOP Object Oriented Programming ���������


//�������õ�����ָ��
//auto_ptr����ֵ����ֵ�Ŀ������캯���͸�ֵ���غ���
//scope_ptrû����ֵ����ֵ�Ŀ������캯���͸�ֵ���غ���
//unique_ptrû����ֵ�Ŀ������캯���͸�ֵ���캯��������ֵ�Ŀ������캯���͸�ֵ���캯��


//������
//mutx û����ֵ����ֵ�Ŀ������캯���͸�ֵ���غ���  û��RAII���ƣ���Ҫ�ֶ�lock unlock
//unique_lock û����ֵ����ֵ�Ŀ������캯���͸�ֵ���غ�����������RAII���Ƶģ����л�ȡ����ʼ��
//lock_guard û����ֵ����ֵ�Ŀ������캯���͸�ֵ���غ�����������RAII���Ƶģ����л�ȡ����ʼ��












//example:
//ThreadPool pool;
//pool.start(4);
// 
//class MyTask:public Task{
//public:
//	//��д�����е�run����
//	void run() {
//		//...
//	}
//};

//pool.submitTask(std::make_shared<MyTask>());












//ʵ���̳߳���ص���
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <thread>
#include <chrono>


//Ϊ�˱���ȫ�������ռ���Ⱦ,��ʵ�ʿ��������в�ȥʹ��using namespace std;
//ʹ�� using namespace �Ὣָ���������ռ��е��������ƶ����뵱ǰ������������ܵ������Ƴ�ͻ��








//c++ �ṩ��class+ö������ ����ֹ����ͬ��ö�����ڲ�ͬ��ö�������г��֣�
// ��ʹ��ö�����ʱ��֪��ʹ���ĸ�ö�����͵���ö����
//�̳߳ص�ģʽ
enum class ThreadPoolMode {
	MODE_FIXED,//�߳������̶�
	MODE_CACHED,//�߳�������̬�仯
};










//c++20�ṩ��semaphore
//ͨ�������������������Զ���ʵ��һ���ź���
class Semaphore {
public:
	//�̹߳黹��Դ����Դ����++
	void post() {
		std::cout << "void post()" << std::endl;
		std::unique_lock<std::mutex> lock(mtx_);
		rescLimit_++;
		//֪ͨ�����������µȴ���ȡ��Դ���߳̿���׼����ȡ������������Դ��
		cv_.notify_all();
	}

	//�̻߳�ȡ��Դ����Դ����--
	void wait() {
		std::unique_lock<std::mutex> lock(mtx_);
		
		//�ж��Ƿ�����Դ��û����Դ�������������µȴ����ȴ��黹��Դ���߳�֪ͨ
		cv_.wait(lock, [this]()->bool {return rescLimit_ > 0; });

		rescLimit_--;
	}
private:
	std::mutex mtx_;//��֤�̻߳���Ļ�����
	std::condition_variable cv_;//�����߳�ͨ�ŵ���������
	int rescLimit_;//��Դ�����ü���
};
















//ʵ��Any���ͽ����������͵ķ���ֵ
//���������������캯��ʵ��Ϊ=default���������ָ����Ż�����ʾΪĬ�ϵ�����������Ĭ�ϵĹ��캯��
class Any {
public:
	
	//Any�Ĺ��캯�������������͵ķ���ֵ
	//ͨ������ֵ������ʵ��������Ӧ�������࣬���ݴ洢�ڶ�Ӧ����������
	//Anyͨ�������ָ�������ܲ�ͬ��������
	template <typename T>
	Any(T data)
		:base_(std::make_unique<Derive<T>>(data))
		//���빹���������Ĳ��������ض���Ķ�ָ��
	{}


	Any() = default;
	~Any() = default;
	Any(const Any&) = delete;
	Any& operator=(const Any&) = delete;
	Any(Any&&) = default;
	Any& operator=(Any&&) = default;


	//�ṩ��Ա�����ӿڸ��û���ȡ����ֵ
	template <typename T>
	T cast_() {
		//������ָ��ת��Ϊ������ָ��ʹ��dynamic_cast;
		Derive<T>* pd = dynamic_cast<Derive<T>*>(base_.get());
		if (pd == nullptr) {
			throw " type is incompatoble !!!";
		}
		return pd->data_;
	}
	//�ⲿ����Է����ڲ���public,private�ĳ�Ա
	//�ڲ������ͨ���ⲿ��Ķ�����Է����ⲿ��ĳ�Ա
private:


	class Base {
	public:
		Base() = default;
		virtual ~Base() = default;
	};


	template <typename T>
	class Derive :public Base {
	public:
		Derive(T data)
			:data_(data) {}
		~Derive() = default;
		T data_;
	};



private:
	std::unique_ptr<Base> base_;
};






class Task;
//ʵ���ύ�����񷵻ص���
//ͨ�����������Ի�ȡ�� ����ִ���귵�ص�Any ,��Any�л�ȡ����ֵ
//�������֧��û��Any��������û��ִ���꣬������ȡ����ֵ
class Result {
public:
	Result(std::shared_ptr<Task> task, bool isValid = true);

	Any get();//��ȡAny�ķ���ֵ

	void setVal(Any any);//�����߳�ִ��������ķ���ֵ

private:

	Any any_;//��������ֵ��Any����
	Semaphore sem_;//ʲôʱ������ִ���꣬�з���ֵ���߳�ͨ��ʹ�õ��ź������źż�¼�ľ��Ǵ��з���ֵany���������
	std::shared_ptr<Task> task_;//ʲô����ķ���ֵ
	std::atomic_bool isValid_;//����ֵ�Ƿ���Ч
};








//�̳߳���Ҫ���û��ṩ�ϴ�����Ľӿڣ������û��������Ƕ��ֶ�����
//ֻ���ṩһ���ӿڣ���ν��ܲ�ͬ�������أ�ͨ����̬��ʵ�֣�
//������ĳ���������������麯�����û�ͨ����������д���麯������ʵ���Լ���Ҫ�̳߳ش��������
//�̳߳ظ��û��ṩ��������Ľӿڣ��ýӿں���ͨ������Ļ���ָ������û������񣬴Ӷ�ʵ�ֶ�̬

class Task {
public:
	Task();
	void exec();
	void setResult(Result* res);
	virtual Any run() = 0;
private:
	Result* result_;
	//����ʹ������ָ�룬����ɽ���������
};






//�߳���
class Thread {
	//���庯��������������
	//ʹ��function����bind���ص��̺߳�������
	using ThreadFunc = std::function<void(int)>;
public:
	//�̹߳��캯������bind���ص��̺߳��������ʼ���̺߳�������
	Thread(ThreadFunc func);
	//�߳���������
	~Thread();
	//�����߳�ִ���̺߳���
	void start();
	//��ȡ�߳�ID
	int getId()const;
private:
	//������̺߳�������
	ThreadFunc func_;
	int threadId_;
	static int generateId_;
};


//�̳߳���
class ThreadPool {
private:
	//Ϊ�߳��ṩ�̺߳���
	//Ϊʲô�̺߳�����������Thread��Ҫ������ThreadPool�У�
	// ��Ϊ�̷߳��ʷ��ʵĹ�����Դ��������У���ThreadPool��
	//�Լ���֤�̰߳�ȫ���߳�ͨ�ŵĻ�����������������ThreadPool��
	//������ThreadPool�з�����ʹ�����Դ��ʹ�û���������������
	//Ϊ�߳��ṩ�̺߳������̺߳���ִ����������е������߳�ͨ���̺߳�����������
	void threadFunc(int threadId);


	//����̳߳ص�����״̬
	bool checkRunningState();

public:
	//�̳߳ع���
	ThreadPool();
	//�̳߳���������
	~ThreadPool();
	//Ϊ�û��ṩ�����̳߳ع���ģʽ�Ľӿ�
	void setMode(ThreadPoolMode mode);

	

	//Ϊ�û��ṩ����������������ֵ�Ľӿڣ��û����Ը����Լ���Ӳ����Դ���к��������
	void setTaskQueMaxThreshHold(int maxThreadHold);

	//cached ģʽ�� Ϊ�û��ṩ�����߳����������ֵ�Ľӿڣ��û����Ը����Լ���Ӳ���������������
	void setThreadSizeMaxThreshHold(int threadhold);


	//Ϊ�û��ṩ�����̳߳صĽӿ�
	//�߳̿������û����Դ����ʼ�̵߳����������ó�ʼ�߳��������û����Ը���Ӳ���������к��������
	//Ĭ����cpu�ĺ���������ʼ�̵߳ĸ���
	void start(int size=std::thread::hardware_concurrency());

	//Ϊ�û��ṩ�ύ�߳�����Ľӿ�
	Result submitTask(std::shared_ptr<Task> sp);


	//�û����ܽ��п����͸���  ��Ϊ������������������֧�ֿ����͸���
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator = (const ThreadPool&) = delete;
private:
	//ʹ��vector�����洢�߳�  ���߳�ָ��ʵ����vector��
	//vector���������Ԫ�ص���������������Thread*��ָ�����ͣ�
	//û������������ֻ���ֶ�deleteָ�룬����ָ��ָ��������������
	//Ϊ�˿����Զ�deleteָ�룬����ʹ������ָ�룬vector����������Ԫ�ص���������
	// Ԫ�ؾ�������ָ�룬����ָ����������������������������delete��װ���߳�ָ��
	// vetor�洢�̵߳�����ָ��
	//std::vector<std::unique_ptr<Thread>> threads_;
	//��Ҫͨ���߳�Id�ҵ���Ӧ���߳�
	std::unordered_map<int, std::unique_ptr<Thread>>threads_;

	//��ʼ�̵߳�����  Ҳ����vector���̵߳ĳ�ʼ����
	int initThreadSize_;


	//�߳������������ֵ,��¼�����Դ������߳�����
	int threadSizeMaxThreshHold_;


	//��¼�����˵��̵߳�������Ҳ����vector���̵߳Ĵ�С
	//����vector�����̰߳�ȫ������ʹ��atomic_int��¼�����̵߳Ĵ�С
	std::atomic_int curThreadSize_;

	//��¼�����̵߳�����,
	
	std::atomic_int idleThreadSize_; //�ڶ��߳���ʹ�ñ�֤�̰߳�ȫʹ��atomic


	//�ö������洢�����û��ύ����ӿ���ʹ�ö�̬ԭ��ʵ�ֵģ�������Ļ���ָ��ʵ����queue��
	//����ʹ�������ָ�벻��ȫ������û��ύ����������ʱ������ô�����ϴ�����������������
	//�ȵ��߳�ȥ��������ʱ����������ָ����һ��Ұָ�룬���ǲ�����ġ�
	//������Ҫʹ�����ܻ���ָ����ʵ����queue��queue�д洢��������ܻ���ָ��
	std::queue<std::shared_ptr<Task>> tasks_;

	//������������������������������б��û��̷߳���Ҳ���̳߳��е��̷߳��ʣ���������������̹߳���ģ�
	// ͬ����еĴ�СҲ���̹߳���ġ�Ϊ�˱�֤�̰߳�ȫ�����д�С���޸�Ӧ����ԭ�ӵģ�������ʹ��atomic_int��֤taskSize��ԭ����
	std::atomic_int taskSize_;

	//������������������������ֵ,��ֹ�ύ��������ཫ�ڴ�ű���
	int taskQueMaxThreshHold_;

	//����������̹߳���ģ�������Ҫ��֤��������������̰߳�ȫ�ģ�ʹ�û�������֤�̰߳�ȫ
	std::mutex taskQueMtx_;

	//Ϊ��ʵ���̼߳�ͨ�ţ���Ҫʹ�����������ͻ����� ��
	// ���������û������ʱ������������߳�Ӧ�ü�����������no�������ͷţ�������ȴ�״̬
	std::condition_variable notFull_;//������в���
	std::condition_variable notEmpty_;//������в���
	ThreadPoolMode poolMode_;//��ǰ�̳߳ص��߳�ģʽ

	std::atomic_bool isPoolRunning_;//��¼�̳߳��Ƿ�������������Ҫ�ڶ���߳���ʹ�ã�����ʹ��atomic
	
	std::condition_variable exitCond_;


};




















#endif


