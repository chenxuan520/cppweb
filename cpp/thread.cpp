#include"../hpp/thread.h"
#include<queue>
#include<pthread.h>
#include"../hpp/server.h"
using namespace std;
void* ThreadPool::worker(void* arg)//the worker for user 
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
void* ThreadPool::manager(void* arg)//manager for user
{
	return NULL;
}
ThreadPool::ThreadPool(unsigned int threadNum)//create threads
{
	if(threadNum<=1)
		threadNum=10;
	thread=new pthread_t[threadNum];
	if(thread==NULL)
		throw NULL;
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
ThreadPool::~ThreadPool()//destory pool
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
void ThreadPool::threadExit()// a no use funtion
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
void ThreadPool::addTask(Task task)//by this you can add task
{
	if(isContinue==false)
		return;
	pthread_mutex_lock(&this->lockPoll);
	this->thingWork.push(task);
	pthread_mutex_unlock(&this->lockPoll);
	pthread_cond_signal(&this->condition);
}
void ThreadPool::endPool()//user delete the pool
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
void ThreadPool::getBusyAndTask(unsigned int* pthread,unsigned int* ptask)//get busy live and task num
{
	pthread_mutex_lock(&lockBusy);
	*pthread=busyThread;
	pthread_mutex_unlock(&lockBusy);
	pthread_mutex_lock(&lockPoll);
	*ptask=thingWork.size();
	pthread_mutex_unlock(&lockPoll);
}
pthread_t ThreadPool::createPthread(void* arg,void* (*pfunc)(void*))//create a thread 
{
	pthread_t thread=0;
	pthread_create(&thread,NULL,pfunc,arg);
	return thread;
}
void ThreadPool::createDetachPthread(void* arg,void* (*pfunc)(void*))//create a ddetach thread
{
	pthread_t thread=0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&thread,&attr,pfunc,arg);
}
/********************************
	author:chenxuan
	date:2021/11/5
	funtion:server tcp ip 2.0
*********************************/
ServerPool::ServerPool(unsigned short port,unsigned int threadNum):ServerTcpIp(port)
{
	if(threadNum>0)
	{	
		pool=new ThreadPool(threadNum);
		if(pool==NULL)
			throw NULL;
	}
	pthread_mutex_init(&mutex,NULL);
}
ServerPool::~ServerPool()
{
	delete pool;
   	pthread_mutex_destroy(&mutex);
}
bool ServerPool::mutexTryLock()
{
	if(0==pthread_mutex_trylock(&mutex))
		return true;
	else
		return false;
}
void ServerPool::threadModel(void* pneed,void* (*pfunc)(void*))
{
	ServerPool::ArgvSer argv={*this,0,pneed};
	ThreadPool::Task task={pfunc,&argv};
	while(1)
	{
		sockaddr_in newaddr={0};
		int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
		if(newClient==-1)
			continue;
		this->addFd(newClient);
		argv.soc=newClient;
		task.ptask=pfunc;
		task.arg=&argv;
		pool->addTask(task);
	}
}
bool ServerPool::epollThread(int* pthing,int* pnum,void* pget,int len,void* pneed,void* (*pfunc)(void*))
{
	int eventNum=epoll_wait(epfd,pevent,512,-1),thing=0,num=0;
	for(int i=0;i<eventNum;i++)
	{
		epoll_event temp=pevent[i];
		if(temp.data.fd==sock)
		{
			sockaddr_in newaddr={0};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
            this->addFd(newClient);
			nowEvent.data.fd=newClient;
			nowEvent.events=EPOLLIN;
			epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
			thing=1;
			num=newClient;
			strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
			ServerPool::ArgvSerEpoll argv={*this,num,thing,0,pneed,pget};
			ThreadPool::Task task={pfunc,&argv};
			if(pfunc!=NULL)
				pool->addTask(task);
		}
		else
		{
			int getNum=recv(temp.data.fd,(char*)pget,len,0);
			num=temp.data.fd;
			if(getNum>0)
				thing=2;
			else
			{
				*(char*)pget=0;
				thing=0;
                this->deleteFd(temp.data.fd);
				epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
				close(temp.data.fd);
			}
			if(pfunc!=NULL)
			{
				ServerPool::ArgvSerEpoll argv={*this,num,thing,getNum,pneed,pget};
				ThreadPool::Task task={pfunc,&argv};
				if(pfunc!=NULL)
					pool->addTask(task);
			}
		}
	}
	return true;
}
