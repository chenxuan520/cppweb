#ifndef _THREAD_H_
#define _THREAD_H_
#include<queue>
#include<pthread.h>
#include"server.h"
/********************************
	author:chenxuan
	date:2021/8/10
	funtion:class thread pool
*********************************/
namespace cppweb{

class ThreadPool{
public://a struct for you to add task
	struct Task{
		void* (*ptask)(void*);
		void* arg;
	};
private:
	std::queue<Task> thingWork;//a queue for struct task
	pthread_cond_t condition;//a condition mutex
	pthread_mutex_t lockPoll;//a lock to lock queue
	pthread_mutex_t lockTask;//a lock for user to ctrl
	pthread_mutex_t lockBusy;//a lock for busy thread
	pthread_t* thread;//an array for thread
	pthread_t threadManager;//thread for manager user to ctrl
	unsigned int liveThread;//num for live thread
	unsigned int busyThread;//num for busy thread
	bool isContinue;//if the pool is continue
private:
	static void* worker(void* arg);//the worker for user 
	static void* manager(void* arg);//manager for user
public:
	ThreadPool(unsigned int threadNum=10);//create threads
	~ThreadPool();//destory pool
	void threadExit();// a no use funtion
	void addTask(Task task);//by this you can add task
	void endPool();//user delete the pool
	void getBusyAndTask(unsigned int* pthread,unsigned int* ptask);//get busy live and task num;
	inline void mutexLock()//user to lock ctrl
	{
		pthread_mutex_lock(&this->lockTask);
	}
	inline void mutexUnlock()//user to lock ctrl
	{
		pthread_mutex_unlock(&this->lockTask);
	}
	static pthread_t createPthread(void* arg,void* (*pfunc)(void*));//create a thread 
	static inline void waitPthread(pthread_t thread,void** preturn=NULL)//wait the thread end
	{
		pthread_join(thread,preturn);
	}
	static void createDetachPthread(void* arg,void* (*pfunc)(void*));//create a ddetach thread
};
/********************************
	author:chenxuan
	date:2021/11/5
	funtion:server tcp ip 2.0
*********************************/
class ServerPool:public ServerTcpIp{
private:
	ThreadPool* pool;
	pthread_mutex_t mutex;
	unsigned int threadNum;
private:
	static void sigCliDeal(int pid);
public:
	struct ArgvSer{
		ServerPool& server;
		int soc;
		void* pneed;
	};
	struct ArgvSerEpoll{
		ServerPool& server;
		int soc;
		int thing;
		int len;
		void* pneed;
		void* pget;	
	};
public:
	ServerPool(unsigned short port,unsigned int threadNum=0);
	~ServerPool();
	inline void mutexLock()
	{
		pthread_mutex_lock(&mutex);
	}
	inline void mutexUnlock()
	{
		pthread_mutex_unlock(&mutex);
	}
	bool mutexTryLock();
	void forkModel(void* pneed,void (*pfunc)(ServerPool&,int,void*));
	void forkEpoll(unsigned int senBufChar,unsigned int recBufChar,void (*pfunc)(ServerPool::Thing,int,int,void*,void*,ServerPool&));
	void threadModel(void* pneed,void* (*pfunc)(void*));
	void epollThread(int* pthing,int* pnum,void* pget,int len,void* pneed,void* (*pfunc)(void*));
	inline bool threadDeleteSoc(int clisoc)
	{
		close(clisoc);
		return this->deleteFd(clisoc);
	}
};
}
#endif
