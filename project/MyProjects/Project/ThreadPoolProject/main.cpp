#include "threadpool.h"

//�������е����񶼲���Ҫ����ֵ��
//������Ҫ����1��30000��he
//���Է�Ϊ��������
//����һ������1��10000�ĺ�
//�����������10001��20000�ĺ�
//������������20001��30000�ĺ�
//�ֱ���������񽻸��̳߳ش��������������ظ����߳�
//���߳̽���������ķ���ֵ������
//������Ҫÿ���������궼��Ҫ����ֵ
//����ÿ������ķ���ֵ���ܲ�һ����Ҫ��ô����أ�
//ͨ��ģ���ǽ�����˵ģ�Ϊ�˽��ܲ�ͬ��������Ҫʹ�ö�̬
//��̬�еĻ����麯����������ģ�庯��
//���������뵽����ʱ��Ҫ����һ���麯��������Ӧ���麯��ָ��
//��Ҫ���巵��ֵ���ͣ����Ի�����麯��������ģ�庯��
//������������������͵����񷵻�ֵ��
//��java��python����Object���������͵Ļ��࣬����ͨ��Object���ܲ�ͬ���͵ķ���ֵ
//���	C++��Ҳ���Լ���Object�Ϳ��Խ���� C++������Object�Ŀ��Խ����������͵�����
//�Ǿ���C++17�ṩ��Any

using uLong = unsigned long long;
class MyTask :public Task {
public:
	MyTask(int begin,int end)
		:begin_(begin),end_(end){}
	Any run() {
		
		std::this_thread::sleep_for(std::chrono::seconds(2));
		
		uLong sum = 0;
		for (uLong i = begin_; i <= end_; ++i)sum += i;
		std::cout << "thread-tid:" << std::this_thread::get_id() << " executed a task over" << std::endl;
		return sum;
	}
private: 
	int begin_;
	int end_;
};

int main() {
	{
		ThreadPool pool;
		pool.setMode(ThreadPoolMode::MODE_CACHED);
		pool.start(2);
		Result res = pool.submitTask(std::make_shared<MyTask>(1, 10000));
		Result res1 = pool.submitTask(std::make_shared<MyTask>(10001, 20000));
		Result res2 = pool.submitTask(std::make_shared<MyTask>(20001, 30000));
		pool.submitTask(std::make_shared<MyTask>(1, 10000));
		pool.submitTask(std::make_shared<MyTask>(10001, 20000));
		pool.submitTask(std::make_shared<MyTask>(20001, 30000));
		std::cout << "main over" << std::endl;
		std::cout << res.get().cast_<uLong>() << std::endl;
		//getchar();
	}
		
		getchar();
	//{
	//	ThreadPool pool;
	//	pool.setMode(ThreadPoolMode::MODE_CACHED);
	//	pool.start(4);
	
	//	Result res1 = pool.submitTask(std::make_shared<MyTask>(10001, 20000));
	//	Result res2 = pool.submitTask(std::make_shared<MyTask>(20001, 30000));
	//	pool.submitTask(std::make_shared<MyTask>(1, 10000));
	//	pool.submitTask(std::make_shared<MyTask>(10001, 20000));
	//	pool.submitTask(std::make_shared<MyTask>(20001, 30000));
	//	uLong ret = 0;
	//	uLong ret1 = res.get().cast_<uLong>();
	//	uLong ret2 = res1.get().cast_<uLong>();
	//	uLong ret3 = res2.get().cast_<uLong>();
	//	std::cout << ret1 + ret2 + ret3 << std::endl;
	//	/*for (int i = 0; i < 10; ++i) {
	//		pool.submitTask(std::make_shared<MyTask>(1,1000));
	//	}*/
	//	//��������࣬�����߳��Ѿ������߳���������ֵ���̶߳������У������귢��isPoolRunning==false 
	//	// ,�̳߳�Ҫ��������������е�����û��ִ����
	//	//�̳߳ؾ������ˣ�����������
	//}
	//
	//getchar();
	//std::this_thread::sleep_for(std::chrono::seconds(10));
	return 0;
}