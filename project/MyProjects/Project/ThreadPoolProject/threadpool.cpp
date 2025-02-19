//�̳߳�������Ա������ʵ��
#include "threadpool.h"
const int TASK_MAX_THRESH = 1024;
const int THREAD_MAX_THRESH = 10;
const int THREAD_MAX_IDLE_TIME = 60;//������ʱ��60��


//�̳߳صĹ���
//Ĭ�Ͻ���ʼ�߳���������Ϊ4����������������Ϊ0��
//��������е������ֵ����Ϊ1024���̳߳صĹ���ģʽĬ��Ϊfixed
//�̳߳�Ĭ��û������
ThreadPool::ThreadPool()
	:initThreadSize_(4)
	, threadSizeMaxThreshHold_(THREAD_MAX_THRESH)
	, taskSize_(0)
	, idleThreadSize_(0)
	, curThreadSize_(0)
	, taskQueMaxThreshHold_(TASK_MAX_THRESH)
	, poolMode_(ThreadPoolMode::MODE_FIXED)
	, isPoolRunning_(false)
{}


//�̳߳ص���������
ThreadPool::~ThreadPool(){
	isPoolRunning_ = false;
	std::unique_lock<std::mutex> lock(taskQueMtx_);
	notEmpty_.notify_all();
	exitCond_.wait(lock, [this]()->bool {return curThreadSize_ == 0; });
}


//�����̳߳صĹ���ģʽ
void ThreadPool::setMode(ThreadPoolMode mode) {
	//����̳߳ش�������״̬��������״̬
	if (checkRunningState())return;
	poolMode_ = mode;
}


//�����̳߳���������е������ֵ
void ThreadPool::setTaskQueMaxThreshHold(int maxThresh) {
	taskQueMaxThreshHold_ = maxThresh;
}



//�����̳߳����߳������������ֵ
void ThreadPool::setThreadSizeMaxThreshHold(int threadhold) {
	//�̳߳��Ѿ����������������߳������������ֵ
	if (checkRunningState())return;
	//�߳�ģʽΪMODE_CACHED��������
	if(ThreadPoolMode::MODE_CACHED==poolMode_)threadSizeMaxThreshHold_ = threadhold;
}


//�̳߳ض����ṩ�ϴ�����Ľӿ�
Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
	//Ҫ���ʹ�����Դ����������Ȼ�ȡ������
	// �����в�������������������������������
	//�������Ѿ����ˣ���Ҫ�����������µȴ��������ͷ��̴߳��ڵȴ�״̬
	// �ȴ������߳��������񣬵ȴ�֪ͨ���в�Ϊ�����Ի�ȡ�����ٴӵȴ�״̬��Ϊ����״̬��ȡ������
	// ��ȡ���ɹ����߳̽�������״̬�����ɹ����������ȴ����ͷŻ�ȡ������
	std::unique_lock<std::mutex> lock(taskQueMtx_);
	/*while (tasks_.size() == taskQueMaxThreshHold_) {
		notFull_.wait(lock);
	}*/


	////Ҳ����ʹ������������wait����һ�����غ���
	//notFull_.wait(lock, [this]()->bool {
	//	return tasks_.size() < taskQueMaxThreshHold_;
	//	});




	//ʵ���û��̵߳ȴ�����1sû�л�ȡ�����������Ϊ�գ����û��̲߳��������ȴ�
	//
	//���������µ� ������Ա����wait wait_for wait_util
	//wait ���������غ�������Ҫ�ȴ�����������ȡ���ŷ��أ�����һֱ�ȴ�
	//wait_for ������Ҫ�ȴ�����������ȡ��������ȴ�һ��ʱ��Σ��ڸö�ʱ��������������ȡ���˾ͷ���true ���򷵻�false
	//wait_util ������Ҫ�ȴ�����������ȡ��������ȴ���һ��ʱ��㣬��δ��ʱ�������������ȡ���˾ͷ���true�����򷵻�false

	bool ret=notFull_.wait_for(lock,
		std::chrono::seconds(1),
		[this]()->bool {return tasks_.size() < taskQueMaxThreshHold_; });
	//��������������û�л�ȡ��
	if(ret==false){
		std::cerr << "taskQue is empty or not get lock!!!" << std::endl;
		return Result(sp,false);
	}




	//��������
	tasks_.emplace(sp);
	taskSize_++;
	//֪ͨ�ڵȴ�������в�Ϊ�����������µ��߳̿���׼����ȡ������������
	notEmpty_.notify_all();




	//cachedģʽ��ʹ�ó�����������С
	//�ύ�����񣬸��ݿ����̵߳������������������Լ������̵߳������Ƿ񵽴��߳������������ֵ�Ƿ���Ҫ�������߳�
	if (poolMode_ == ThreadPoolMode::MODE_CACHED
		&& idleThreadSize_ < taskSize_ 
		&& curThreadSize_ < threadSizeMaxThreshHold_) {
		std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this,std::placeholders::_1));
		int threadId = ptr->getId();
		threads_.emplace(threadId,std::move(ptr));
		threads_[threadId]->start();//�����߳�
		curThreadSize_++;
		idleThreadSize_++;
		std::cout << "--------------------main thread create a new thread!!!!" << std::endl;
	}




	//���ص������Ӧ��Result����	
	//����һ Task�а���Result���� return task->getResult();
	//������ Result�а���Task  return Result(task);
	//����ʹ�÷���һ��Ϊ�������߳���������ʱ���ڷ���һ�е�task�ᱻ�Ӷ�����pop����task�ᱻ��������task�е�Result��ȻҲ��������
	return Result(sp);

}


//Ϊ�߳��ṩ���̺߳���,�̺߳���ִ����������е�������������
void ThreadPool::threadFunc(int threadId) {
	////std::cout << "ThreadFunc start... tid=" << std::this_thread::get_id() <<std::endl;
	////
	////�����߳�����һ������ִ����һ��������Ҫ����ִ������������������Ҫѭ��


	////��ȡ�̵߳Ŀ�ʼ����ʱ��㣨û��ִ�������ʱ��㣩
	//auto lastTime = std::chrono::high_resolution_clock().now();
	//while(isPoolRunning_==true) {

	//	//���ʹ�����Դ���������Ҫ��ȡ��������֤�̰߳�ȫ


	//	std::shared_ptr<Task> task;

	//	{

	//		


	//		std::unique_lock <std::mutex> lock(taskQueMtx_);
	//		std::cout << "thread-tid:" << std::this_thread::get_id() << "try to get a task." << std::endl;

	//		//�߳�ÿִ����һ�����񣬼���Ƿ��г�ʱ�Ŀ����߳���Ҫ����
	//		//cachedģʽ�£��������̹߳��࣬�����кܶ೬��60sû�л�ȡ������߳�
	//		//��Ҫ����Щ�߳����ٵ�
	//		//��û������
	//		while (isPoolRunning_==true&&taskSize_ == 0) {
	//			//�ȴ����������ȡ����wait���������ֽ��
	//			//1.��ʱ���أ�ÿ��1�뷵��һ�μ���Ƿ��г�ʱ�����߳�
	//			//2.���������һ�ȡ������
	//			if (poolMode_ == ThreadPoolMode::MODE_CACHED) {
	//				if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1))) {
	//					//��ʱ����
	//					auto now = std::chrono::high_resolution_clock().now();
	//					auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
	//					if (dur.count() > THREAD_MAX_IDLE_TIME
	//						&& curThreadSize_ > initThreadSize_) {
	//						//��ǰ�߳̿��г�ʱ������
	//						//����ǰ�߳���Ϣ���߳��б���ɾ��
	//						//�޸��߳������йصı���
	//						threads_.erase(threadId);
	//						curThreadSize_--;
	//						idleThreadSize_--;
	//						std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
	//						//�̺߳���ִ���߳̾��ͷ���
	//						return;

	//					}
	//				}
	//				//else {
	//				//	std::cout << "thread-tid:" << std::this_thread::get_id() << "get lock success." << std::endl;
	//				//	if (taskSize_ > 0) {
	//				//		std::cout << "thread-tid:" << std::this_thread::get_id() << "get lock success. go to exec task" << std::endl;
	//				//		break;
	//				//	}
	//				//}
	//				////else break;
	//			}
	//			else {
	//				//��������в�Ϊ�գ�����ִ�ж��У����Ѷ���
	//				//������Ϊ�գ���Ҫ����������notEmpty�µȴ�������в�Ϊ�գ������߳̽���֪ͨ
	//				//���������µȴ������߳̽����ͷţ������߳̽���ȴ�״̬
	//				//�����߳�����������֪ͨ�����߳����ѣ������߳��ɵȴ�״̬��Ϊ����״̬������״̬���Ի�ȡ������
	//				//��û�л�ȡ�����������������ȡ������������ȡ�������ˣ��������״̬���ٵ�����״̬��������������

	//				notEmpty_.wait(lock);

	//			}
	//			//�̳߳ػ����ͷ���Դ
	//			/*if (isPoolRunning_ == false) {
	//				threads_.erase(threadId);
	//				curThreadSize_--;
	//				idleThreadSize_--;
	//				std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
	//				if (curThreadSize_ == 0)exitCond_.notify_all();
	//				return;
	//			}*/
	//			
	//		}
	//		
	//		if (isPoolRunning_ == false) {
	//			threads_.erase(threadId);
	//			curThreadSize_--;
	//			idleThreadSize_--;
	//			std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
	//			if (curThreadSize_ == 0)exitCond_.notify_all();
	//			return;
	//		}




	//		//else {
	//		//	//��������в�Ϊ�գ�����ִ�ж��У����Ѷ���
	//		////������Ϊ�գ���Ҫ����������notEmpty�µȴ�������в�Ϊ�գ������߳̽���֪ͨ
	//		////���������µȴ������߳̽����ͷţ������߳̽���ȴ�״̬
	//		////�����߳�����������֪ͨ�����߳����ѣ������߳��ɵȴ�״̬��Ϊ����״̬������״̬���Ի�ȡ������
	//		////��û�л�ȡ�����������������ȡ������������ȡ�������ˣ��������״̬���ٵ�����״̬��������������

	//		//	notEmpty_.wait(lock,
	//		//		[this]()->bool {return tasks_.size() > 0; });

	//		//}







	//		
	//		//�߳�״̬�ı䣬�������̵߳�����--
	//		idleThreadSize_--;



	//		//�������񣬽�����Ӷ������ó���
	//		task = tasks_.front();
	//		tasks_.pop();
	//		taskSize_--;

	//		std::cout << "thread-tid:" << std::this_thread::get_id() << " get a task success." << std::endl;


	//		//�������ѣ�������Ӷ�����ȡ���������ֻ�����������л�������
	//		//֪ͨ�ȴ���notEmpty���������µĵȴ�������в�Ϊ�յ������̣߳���������׼����ȡ������
	//		if (tasks_.size() > 0) {
	//			notEmpty_.notify_all();
	//		}


	//		//֪ͨ�����̣߳�������в�Ϊ�գ�����׼����ȡ������������
	//		notFull_.notify_all();


	//		//�������������߳����ͷ�
	//	}

	//	//�ڳ�ִ������֮ǰ���������ͷ�
	//	//Ϊʲô��
	//	//��Ϊ���Ᵽ֤�̰߳�ȫ�ķ���������У�ִ������ǰ�Ѿ�������Ӷ�����ȡ����
	//	//���ٷ���������У�û��Ҫ����ռ����
	//	//ԭ��2���ִ������֮ǰ�������ͷţ���ִ������Ĺ����������̲߳��ܻ�ȡ�������ܴ���������н�����ȡ����
	//	//���������̲߳��ܽ�����ȡ�������϶��Ͳ���ִ������������ֻ��һ�������߳���ִ������
	//	//Ҳ����cpu��ֻ��һ�����߳���ִ�У��������cpu�ж���ˣ�cpu��Ч��̫�ͣ������ϸ߲�����Ŀ��




	//	//�����߳�ִ������
	//	//���ݻ���ָ��ָ�������������ִ������������
	//	if(task.get()!=nullptr)task->exec();
	//	std::cout << "thread exec over" << std::endl;


	//	//�߳�ִ��������״̬�����ı�,�����̵߳�����++
	//	idleThreadSize_++;

	//	//����
	//	if (isPoolRunning_ == false) {
	//		threads_.erase(threadId);
	//		curThreadSize_--;
	//		//idleThreadSize_--;
	//		std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
	//		if (curThreadSize_ == 0)exitCond_.notify_all();
	//		return;
	//	}
	//}
	//std::cout << "ThreadFunc start... tid=" << std::this_thread::get_id() <<std::endl;
	//
	//�����߳�����һ������ִ����һ��������Ҫ����ִ������������������Ҫѭ��













/////////////////////////////////////////
//��������е�����ִ��������������




	//��ȡ�̵߳Ŀ�ʼ����ʱ��㣨û��ִ�������ʱ��㣩
	auto lastTime = std::chrono::high_resolution_clock().now();
	for(;;) {

		//���ʹ�����Դ���������Ҫ��ȡ��������֤�̰߳�ȫ


		std::shared_ptr<Task> task;

		{

			


			std::unique_lock <std::mutex> lock(taskQueMtx_);
			std::cout << "thread-tid:" << std::this_thread::get_id() << "try to get a task." << std::endl;

			//�߳�ÿִ����һ�����񣬼���Ƿ��г�ʱ�Ŀ����߳���Ҫ����
			//cachedģʽ�£��������̹߳��࣬�����кܶ೬��60sû�л�ȡ������߳�
			//��Ҫ����Щ�߳����ٵ�
			//��û������
			while (taskSize_ == 0) {
				if (isPoolRunning_ == false) {
					threads_.erase(threadId);
					curThreadSize_--;
					//idleThreadSize_--;
					std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
					if (curThreadSize_ == 0)exitCond_.notify_all();
					return;
				}
				//�ȴ����������ȡ����wait���������ֽ��
				//1.��ʱ���أ�ÿ��1�뷵��һ�μ���Ƿ��г�ʱ�����߳�
				//2.���������һ�ȡ������
				if (poolMode_ == ThreadPoolMode::MODE_CACHED) {
					if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1))) {
						//��ʱ����
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
						if (dur.count() > THREAD_MAX_IDLE_TIME
							&& curThreadSize_ > initThreadSize_) {
							//��ǰ�߳̿��г�ʱ������
							//����ǰ�߳���Ϣ���߳��б���ɾ��
							//�޸��߳������йصı���
							threads_.erase(threadId);
							curThreadSize_--;
							idleThreadSize_--;
							std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
							//�̺߳���ִ���߳̾��ͷ���
							return;

						}
					}
					//else {
					//	std::cout << "thread-tid:" << std::this_thread::get_id() << "get lock success." << std::endl;
					//	if (taskSize_ > 0) {
					//		std::cout << "thread-tid:" << std::this_thread::get_id() << "get lock success. go to exec task" << std::endl;
					//		break;
					//	}
					//}
					////else break;
				}
				else {
					//��������в�Ϊ�գ�����ִ�ж��У����Ѷ���
					//������Ϊ�գ���Ҫ����������notEmpty�µȴ�������в�Ϊ�գ������߳̽���֪ͨ
					//���������µȴ������߳̽����ͷţ������߳̽���ȴ�״̬
					//�����߳�����������֪ͨ�����߳����ѣ������߳��ɵȴ�״̬��Ϊ����״̬������״̬���Ի�ȡ������
					//��û�л�ȡ�����������������ȡ������������ȡ�������ˣ��������״̬���ٵ�����״̬��������������

					notEmpty_.wait(lock);

				}
				//�̳߳ػ����ͷ���Դ
				/*if (isPoolRunning_ == false) {
					threads_.erase(threadId);
					curThreadSize_--;
					idleThreadSize_--;
					std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
					if (curThreadSize_ == 0)exitCond_.notify_all();
					return;
				}*/
				
			}
			
			/*if (isPoolRunning_ == false) {
				threads_.erase(threadId);
				curThreadSize_--;
				idleThreadSize_--;
				std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
				if (curThreadSize_ == 0)exitCond_.notify_all();
				return;
			}*/




			//else {
			//	//��������в�Ϊ�գ�����ִ�ж��У����Ѷ���
			////������Ϊ�գ���Ҫ����������notEmpty�µȴ�������в�Ϊ�գ������߳̽���֪ͨ
			////���������µȴ������߳̽����ͷţ������߳̽���ȴ�״̬
			////�����߳�����������֪ͨ�����߳����ѣ������߳��ɵȴ�״̬��Ϊ����״̬������״̬���Ի�ȡ������
			////��û�л�ȡ�����������������ȡ������������ȡ�������ˣ��������״̬���ٵ�����״̬��������������

			//	notEmpty_.wait(lock,
			//		[this]()->bool {return tasks_.size() > 0; });

			//}







			
			//�߳�״̬�ı䣬�������̵߳�����--
			idleThreadSize_--;



			//�������񣬽�����Ӷ������ó���
			task = tasks_.front();
			tasks_.pop();
			taskSize_--;

			std::cout << "thread-tid:" << std::this_thread::get_id() << " get a task success." << std::endl;


			//�������ѣ�������Ӷ�����ȡ���������ֻ�����������л�������
			//֪ͨ�ȴ���notEmpty���������µĵȴ�������в�Ϊ�յ������̣߳���������׼����ȡ������
			if (tasks_.size() > 0) {
				notEmpty_.notify_all();
			}


			//֪ͨ�����̣߳�������в�Ϊ�գ�����׼����ȡ������������
			notFull_.notify_all();


			//�������������߳����ͷ�
		}

		//�ڳ�ִ������֮ǰ���������ͷ�
		//Ϊʲô��
		//��Ϊ���Ᵽ֤�̰߳�ȫ�ķ���������У�ִ������ǰ�Ѿ�������Ӷ�����ȡ����
		//���ٷ���������У�û��Ҫ����ռ����
		//ԭ��2���ִ������֮ǰ�������ͷţ���ִ������Ĺ����������̲߳��ܻ�ȡ�������ܴ���������н�����ȡ����
		//���������̲߳��ܽ�����ȡ�������϶��Ͳ���ִ������������ֻ��һ�������߳���ִ������
		//Ҳ����cpu��ֻ��һ�����߳���ִ�У��������cpu�ж���ˣ�cpu��Ч��̫�ͣ������ϸ߲�����Ŀ��




		//�����߳�ִ������
		//���ݻ���ָ��ָ�������������ִ������������
		if(task.get()!=nullptr)task->exec();
		std::cout << "thread exec over" << std::endl;


		//�߳�ִ��������״̬�����ı�,�����̵߳�����++
		idleThreadSize_++;

		//����
		//if (isPoolRunning_ == false) {
		//	threads_.erase(threadId);
		//	curThreadSize_--;
		//	//idleThreadSize_--;
		//	std::cout << "threadId:" << std::this_thread::get_id() << "exit!!!" << std::endl;
		//	if (curThreadSize_ == 0)exitCond_.notify_all();
		//	return;
		//}
	}
	
}


//�����̳߳�
//�߳̿������û����Դ����ʼ�̵߳����������ó�ʼ�߳��������û����Ը���Ӳ���������к��������
void ThreadPool::start(int initThreadSize){
	//�̳߳�������
	isPoolRunning_ = true;
	//��¼��ʼ�̵߳ĸ���
	initThreadSize_ = initThreadSize;

	curThreadSize_ = initThreadSize_;

	//�̵߳Ĵ���
	//�����߳���Ҫ�ṩ�߳���Ҫִ�е��̺߳���
	//�����̺߳�����ThreadPool�У������̺߳�����ҪThreadPool�������ָ��
	//������Thread��û��ThreadPool�����ָ�룬����ô����
	//ͨ��bind������TreadPool��thisָ��󶨵��̺߳�����
	//������Thread�о��ܵ�����ThreadPool�е��̺߳�����
	//make_unique������ʱ��unique_ptr����
	//unique_ptrû����ֵ�Ŀ������캯��
	//ֻ����ֵ�Ŀ������캯��
	//���Զ�ptr��move
	for (int i = 0; i < initThreadSize_; ++i) {
		std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this,std::placeholders::_1));
		//threads_.emplace_back(std::move(ptr));
		int threadId=ptr->getId();
		threads_.emplace(threadId, std::move(ptr));
	}
	//�����߳�,����һ���̶߳���ȥִ���̺߳���
	for (int i = 0; i < initThreadSize_; ++i) {
		threads_[i]->start();
		//��¼�����̵߳�����
		idleThreadSize_++;

	}
}



//����̳߳ص�����״̬
bool ThreadPool::checkRunningState() { return isPoolRunning_; }






//�����̳߳�Ա������ʵ��





//�̵߳Ĺ��캯��
//��function�����̺߳�������

//��̬��Ա������ʼ��

int Thread::generateId_ = 0;


int Thread::getId()const {
	return threadId_;
}

Thread::Thread(ThreadFunc func)
	:func_(func)
	,threadId_(generateId_++)
{}

//�̵߳���������
Thread::~Thread(){}

//�����̶߳���ȥִ���̺߳���
//�߳�ȥִ���̺߳���
void Thread::start() {
	std::thread t(func_,threadId_);
	//��ֹ�̶߳�������������������̶߳��������̷߳���
	t.detach();
}










//��������Result�ķ�����ʵ��
Result::Result(std::shared_ptr<Task> task, bool isValid)
	:task_(task), isValid_(isValid)
{
	task_->setResult(this);
}


Any Result::get() {
	if (isValid_ == false) {
		return "";
	}
	sem_.wait();//task�������û��ִ���꣬�û��̻߳���	��������ס
	//any�����unique_ptr<Base>û����ֵ�Ŀ������캯����������Ҫʹ��move����ֵתΪ��ֵ������ֵ���ÿ���������unique������ֵ���ÿ�������
	return std::move(any_);
}


void Result::setVal(Any any) {
	//�����߳�ִ�������񽫷�װ�˷���ֵ��any���󷵻�
	this->any_ = std::move(any);
	//��ȡ����ȡ���з���ֵ��any���󣬶��ź���++��
	//�źż�¼�ľ��Ƿ���ֵ������
	sem_.post();
}





/////////////////////////////////////////////////////////////////////////////

Task::Task():result_(nullptr){}

void Task::exec() {
	if (result_ != nullptr)
	{
		result_->setVal(run());
	}
}


void Task::setResult(Result* res) {
	result_ = res;
}








//////////////////////////////////////////////////////////////////////////
// Ӧ���и���������������ύ����������ͨ����������ܻ�ȡ����ķ���ֵ
// ��������������Ҫʵ��һ�£��������������ͽ���Result
// �����Result������Task����������������������Task�����ж���Result����
// Ҳ������Result�����ж���Task���������ַ��������Խ�������������������������
// ѡ�����ַ������ҿ�����ķ���
// ǰ��ͨ��Task�����ȡ������󣬺���ֱ�ӷ��ؽ������
// Ӧѡ����Result�ж���Task������Result����
// �����Task�ж���Result,Task�������ִ���������,������ֵ�洢��Result�к�,���������ˣ�Taskһ�����꣬��Ա����Result�Ϳ�ʼ����
// �ⲿ��Result��Task�ɶ���ͬһ��Result���������Task��Result����Ŀ�����
// ����Task�����Ὣ��Ա����Ҳ�������ⲿ�Ͳ��ܷ����������Ķ���	
// 
//�ύ���񷵻ؽ������
// �������Ӧ��Ƕ����shared_ptr<Task>�л���shared_ptr<Task> Ƕ���ڽ��������
// 
//���ڷ���ֵ�����Ͳ�ȷ����������Ҫ������ֵ��װ��ͳһ���͵Ķ�����з���
//�������ύʧ�ܣ�ͨ����������ȡ����ֵ����Ӧ�������ȴ��߳�ִ�������񷵻ؽ������Ϊ����û���ύ����ɹ���
//ͨ���������Ӧ���������ؿա�������α�ʶ�ύ����û���гɹ��أ�ͨ����������е�isValid��ʶ










///////////////////////////////////////////////////////////////////////
//����cachedģʽ����������
// cachedģʽӦ����������С�ĳ����£���������ʱ�䳤���߳�Ӧ��ʹ��fixedģʽ
//�������Ӧ�ô�����������߳�
//�������̹߳��࣬�߳̿���ʱ�䳬��60s��������̻߳��յ�
//���ڿ����߳������Ĵ���
//���ύ����Ľӿں����У����ݿ����̵߳�����������������������Ƿ���Ҫ�����µ��߳�







/////////////////////////
//�����������������л��������ǵ���������е�����ȫ��ִ����ɲŽ��������ǲ�ִ��ʣ�µ������ˣ�


