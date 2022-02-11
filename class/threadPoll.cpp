/********************************
	author:chenxuan
	date:2021.7.14
	funtion:this file is a try for thead pool
*********************************/
#include<pthread.h>
#include<stdio.h>
#include<iostream>
#include<queue>
#include<vector>
#include<unistd.h>
#include<thread>
#include<mutex>
#include<atomic>
#include<condition_variable>
using namespace std;
/********************************
	author:chenxuan
	date:2021.7.17
	funtion:this is a class for thread pool
*********************************/
class ThreadPool{
public://a struct for you to add task
	struct Task{
		void* (*ptask)(void*);
		void* arg;
	};
private:
	mutex queLock;
	condition_variable condition;
	queue<Task> que;
	unsigned threadNum;
	atomic<unsigned> busyNow;
	atomic<bool> isContinue;
	vector<thread*> arr; 
	const char* error;
public:
	ThreadPool(unsigned threadNum=10)
	{
		error=NULL;
		this->threadNum=threadNum;
		for(unsigned i=0;i<threadNum;i++)
			arr.push_back(new thread(worker,this));
		isContinue=true;
		busyNow=0;
	}
	~ThreadPool()
	{
		stopPool();
		for(unsigned i=0;i<arr.size();i++)
		{
			arr[i]->join();
			delete arr[i];
		}
	}
	void addTask(Task task)
	{
		unique_lock<mutex> guard(this->queLock);
		this->que.push(task);
		this->condition.notify_one();
	}
	void stopPool()
	{
		isContinue=false;
		this->condition.notify_all();
	}
	void getBusyAndTask(unsigned& busy,unsigned& task)
	{
		unique_lock<mutex> guard(this->queLock);
		task=this->que.size();
		busy=this->busyNow;
	}
private:
	static void worker(void* arg)
	{
		ThreadPool& pool=*(ThreadPool*)arg;
		Task task={0,0};
		while(1)
		{
			{
				unique_lock<mutex> guard(pool.queLock);
				if(pool.isContinue&&pool.que.size()==0)
					pool.condition.wait(guard,[&]()->bool{
										return pool.que.size()>0||pool.isContinue==false;
										});
				if(pool.isContinue==false)
					return ;
				if(pool.que.size()==0)
					continue;
				task=pool.que.front();
				pool.que.pop();
			}
			pool.busyNow++;
			if(task.ptask!=NULL)
				task.ptask(task.arg);
			pool.busyNow--;
		}
	}
};
int temp=0;
void* print(void* pha)
{
	/* unsigned int pid=(unsigned)this_thread::get_id(); */
	temp++;
	*(int*)pha+=1;
//	cout<<pid<<":"<<temp<<endl;
	/* cout<<temp<<" "<<this_thread::get_id()<<" ing"<<endl; */
	/* printf("%u:%d ing\n",pid,temp); */
	printf("%d %d\n",(int)temp,*(int*)pha);
	usleep(3);
	return NULL;
}
int main()
{
	ThreadPool pool(10);
	int ha=0;
	ThreadPool::Task task{print,&ha};
	unsigned int thread=1;
	unsigned int busy=0;
	for(unsigned int i=0;i<1000;i++)
	{
		pool.addTask(task);
	}
	while(thread>0)
	{
		pool.getBusyAndTask(thread,busy);
		printf("%u busy %u task\n",thread,busy);
		usleep(7);
	}
	sleep(2);
	return 0;
}
