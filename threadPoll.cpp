/********************************
	author:chenxuan
	date:2021.7.14
	funtion:this file is a try for thead poll
*********************************/
#include<pthread.h>
#include<stdio.h>
#include<iostream>
#include<queue>
#include<unistd.h>
using namespace std;
/********************************
	author:chenxuan
	date:2021.7.17
	funtion:this is a class for thread pool
*********************************/
class ThreadPool{
public:
	struct Task{
		void* (*ptask)(void*);
		void* arg;
	};
private:
	queue<Task> thingWork;
	pthread_cond_t condition;
	pthread_mutex_t lockPoll;
	pthread_mutex_t lockTask;
	pthread_mutex_t lockBusy;
	pthread_t* thread;
	pthread_t threadManager;
	unsigned int liveThread;
	unsigned int busyThread;
	bool isContinue;
private:
	static void* worker(void* arg)
	{
		ThreadPool* poll=(ThreadPool*)arg;
		while(1)
		{
			pthread_mutex_lock(&poll->lockPoll);
			while(poll->isContinue==true&&poll->thingWork.size()==0)
				pthread_cond_wait(&poll->condition,&poll->lockPoll);
			if(poll->isContinue==false)
			{
				pthread_mutex_unlock(&poll->lockPoll);
				pthread_exit(NULL);
			}
			if(poll->thingWork.size()>0)
			{
				pthread_mutex_lock(&poll->lockBusy);
				poll->busyThread++;
				pthread_mutex_unlock(&poll->lockBusy);
				ThreadPool::Task task=poll->thingWork.front();
				poll->thingWork.pop();
				pthread_mutex_unlock(&poll->lockPoll);
				task.ptask(task.arg);
				pthread_mutex_lock(&poll->lockBusy);
				poll->busyThread--;
				pthread_mutex_unlock(&poll->lockBusy);
			}
		}
		return NULL;
	}
	static void* manager(void* arg)
	{
		return NULL;
	}
public:
	ThreadPool(unsigned int threadNum=10)
	{
		if(threadNum<=1)
			threadNum=10;
		thread=new pthread_t[threadNum];
		if(thread==NULL)
			exit(0);
		for(unsigned int i=0;i<threadNum;i++)
			thread[i]=0;
		pthread_cond_init(&condition,NULL);
		pthread_mutex_init(&lockPoll,NULL);
		pthread_mutex_init(&lockTask,NULL);
		pthread_mutex_init(&lockBusy,NULL);
		liveThread=threadNum;
		isContinue=true;
		busyThread=0;
		pthread_create(&threadManager,NULL,manager,this);
		for(unsigned int i=0;i<threadNum;i++)
			pthread_create(&thread[i],NULL,worker,this);
	}
	~ThreadPool()
	{
		if(isContinue==false)
			return;
		isContinue=false;
		pthread_join(threadManager,NULL);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_cond_signal(&condition);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_join(thread[i],NULL);
		pthread_cond_destroy(&condition);
		pthread_mutex_destroy(&lockPoll);
		pthread_mutex_destroy(&lockTask);
		pthread_mutex_destroy(&lockBusy);
		delete[] thread;
	}
	void threadExit()
	{
		pthread_t pid=pthread_self();
		for(unsigned int i=0;i<liveThread;i++)
			if(pid==thread[i])
			{
				thread[i]=0;
				break;
			}
		pthread_exit(NULL);
	}
	void addTask(Task task)
	{
		if(isContinue==false)
			return;
		pthread_mutex_lock(&this->lockPoll);
		this->thingWork.push(task);
		pthread_mutex_unlock(&this->lockPoll);
		pthread_cond_signal(&this->condition);
	}
	void endPool()
	{
		isContinue=false;
		pthread_join(threadManager,NULL);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_cond_signal(&condition);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_join(thread[i],NULL);
		pthread_cond_destroy(&condition);
		pthread_mutex_destroy(&lockPoll);
		pthread_mutex_destroy(&lockTask);
		delete[] thread;
	}
	void getBusyAndTask(unsigned int* pthread,unsigned int* ptask)
	{
		pthread_mutex_lock(&lockBusy);
		*pthread=busyThread;
		pthread_mutex_unlock(&lockBusy);
		pthread_mutex_lock(&lockPoll);
		*ptask=thingWork.size();
		pthread_mutex_unlock(&lockPoll);
	}
	inline void mutexLock()
	{
		pthread_mutex_lock(&this->lockTask);
	}
	inline void mutexUnlock()
	{
		pthread_mutex_unlock(&this->lockTask);
	}
};
int temp=0;
void* print(void*)
{
	unsigned int pid=pthread_self();
	temp++;
//	cout<<pid<<":"<<temp<<endl;
	printf("%u:%d ing\n",pid,temp);
	usleep(3);
	return NULL;
}
int main()
{
	ThreadPool poll(10);
	ThreadPool::Task task{print,NULL};
	unsigned int thread=0;
	unsigned int busy=0;
	for(unsigned int i=0;i<100;i++)
		poll.addTask(task);
	for(int i=0;i<20;i++)
	{
		poll.getBusyAndTask(&thread,&busy);
		printf("%u busy %u task\n",thread,busy);
		usleep(7);
	}
	sleep(5);
	return 0;
}
