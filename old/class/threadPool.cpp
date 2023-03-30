/********************************
	author:chenxuan
	date:2021.7.14
	funtion:this file is a try for thead pool
*********************************/
#include<pthread.h>
#include<string.h>
#include<stdio.h>
#include<iostream>
#include<queue>
#include<vector>
#include<unistd.h>
#include<thread>
#include<mutex>
#include<semaphore.h>
#include<atomic>
#include<condition_variable>
using namespace std;
/********************************
	author:chenxuan
	date:2021.7.17
	funtion:this is a class for thread pool
*********************************/
template<class T=void*>
class ThreadPool{
public://a struct for you to add task
	struct Task{
		void* (*ptask)(T);
		T arg;
	};
private:
	mutex queLock;
	mutex userLock;
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
	inline void mutexLock()
	{
		userLock.lock();
	}
	inline void mutexUnlock()
	{
		userLock.unlock();
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
class LogSystem{
private:
	struct BufferList{
		char* buffer;
		BufferList* pnext;
	};
private:
	const char* fileName;
	BufferList* buffer;
	BufferList* nowBuffer;
	BufferList* saveBuffer;
	const char* error;
	size_t nowLen;
	size_t bufferLen;
	pthread_t pid;
	bool isBusy;
	bool isContinue;
	sem_t sem;
public:
	LogSystem(const char* name,size_t bufferLen=1024):fileName(name)
	{
		nowLen=0;
		pid=0;
		nowBuffer=NULL;
		error=NULL;
		saveBuffer=NULL;
		if(bufferLen<128)
			bufferLen=128;
		if(fileName==NULL)
			fileName="access.log";
		this->bufferLen=bufferLen;
		buffer=(BufferList*)malloc(sizeof(BufferList));
		if(buffer==NULL)
		{
			error="malloc wrong";
			return;
		}
		buffer->buffer=(char*)malloc(sizeof(char)*bufferLen);
		if(buffer->buffer==NULL)
		{
			error="malloc wrong";
			return;
		}
		buffer->pnext=NULL;
		BufferList* last=buffer;
		for(unsigned i=0;i<4;i++)
		{
			last->pnext=(BufferList*)malloc(sizeof(BufferList));
			if(last->pnext==NULL)
			{
				error="malloc wrong";
				return;
			}
			last=last->pnext;
			last->buffer=(char*)malloc(sizeof(char)*bufferLen);
			last->pnext=NULL;
			if(last->buffer==NULL)
			{
				error="malloc wrong";
				return;
			}
			memset(last->buffer,0,sizeof(char)*bufferLen);
		}
		last->pnext=buffer;
		nowBuffer=buffer;
		saveBuffer=nowBuffer;
		sem_init(&sem,1,0);
		isBusy=false;
		isContinue=true;
		/* pid=ThreadPool::createPthread(this,worker); */
	}
	~LogSystem()
	{
		isContinue=false;
		sem_post(&sem);
		/* if(pid!=0) */
		/* 	ThreadPool::waitPthread(pid); */
		sem_destroy(&sem);
		nowBuffer=nowBuffer->pnext;
		BufferList* last=buffer;
		while(last!=NULL)
		{
			BufferList* temp=last;
			if(last->buffer!=NULL)
				free(last->buffer);
			if(last->pnext!=buffer)
			{
				last=last->pnext;
				free(temp);
			}
			else
			{
				free(last);
				break;
			}
		}
	}
public:
	inline const char* lastError()
	{
		return error;
	}
	void accessLog(const char* text)
	{
		if(strlen(text)+nowLen>bufferLen)
		{
			if(nowBuffer->pnext==saveBuffer)
			{
				if(!isBusy)
					sem_post(&sem);
				while(nowBuffer->pnext==saveBuffer);
				nowBuffer=nowBuffer->pnext;
				memset(nowBuffer->buffer,0,sizeof(char)*bufferLen);
				nowLen=0;
			}
			else
			{
				nowBuffer=nowBuffer->pnext;
				memset(nowBuffer->buffer,0,sizeof(char)*bufferLen);
				nowLen=0;
				if(!isBusy)
					sem_post(&this->sem);
			}
		}
		strcat(nowBuffer->buffer,text);
		strcat(nowBuffer->buffer,"\n");
		nowLen+=strlen(text)+1;
	}
	/* static void recordRequest(const void* text,int soc) */
	/* { */
	/* 	static char method[32]={0},askPath[256]={0},buffer[512]={0}; */
	/* 	static LogSystem loger("access.log"); */
	/* 	int port=0; */
	/* 	sscanf((char*)text,"%32s%256s",method,askPath); */
	/* 	time_t now=time(NULL); */
	/* 	sprintf(buffer,"%s %s %s %s",ctime(&now),ServerTcpIp::getPeerIp(soc,&port),method,askPath); */
	/* 	loger.accessLog(buffer); */
	/* } */
private:
	static void* worker(void* argv)
	{
		LogSystem& self=*(LogSystem*)argv;
		FILE* fp=fopen(self.fileName,"a+");
		if(fp==NULL)
		{
			self.error="open file wrong";
			return NULL;
		}
		while(self.isContinue)
		{
			sem_wait(&self.sem);
			self.isBusy=true;
			while(self.saveBuffer!=self.nowBuffer)
			{
				fprintf(fp,"%s",self.saveBuffer->buffer);
				memset(self.saveBuffer->buffer,0,sizeof(char)*self.bufferLen);
				self.saveBuffer=self.saveBuffer->pnext;
			}
			self.isBusy=false;
		}
		fprintf(fp,"%s",self.saveBuffer->buffer);
		fclose(fp);
		return NULL;
	}
};
int temp=0;
void* print(int arg)
{
	/* unsigned int pid=(unsigned)this_thread::get_id(); */
	temp++;
//	cout<<pid<<":"<<temp<<endl;
	/* cout<<temp<<" "<<this_thread::get_id()<<" ing"<<endl; */
	/* printf("%u:%d ing\n",pid,temp); */
	printf("arg %d temp %d\n",arg,temp);
	usleep(3);
	return NULL;
}
int main()
{
	ThreadPool<int> pool(10);
	int ha=0;
	ThreadPool<int>::Task task{print,ha};
	unsigned int thread=1;
	unsigned int busy=0;
	for(unsigned int i=0;i<1000;i++)
	{
		task.arg++;
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
