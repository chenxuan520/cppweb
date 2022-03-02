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
class LogSystem{
private:
	const char* fileName;
	char* buffer[4];
	const char* error;
	char* now;
	char* page;
	size_t nowLen;
	size_t bufferLen;
	ThreadPool pool;
	std::queue<char*> nowFree;
public:
	LogSystem(const char* name,size_t bufferLen=1024)\
										   :fileName(name),now(NULL),pool(1)
	{
		page=NULL;
		if(fileName==NULL)
			fileName="access.log";
		if(bufferLen<128)
			bufferLen=128;
		this->bufferLen=bufferLen;
		for(unsigned i=0;i<4;i++)
			buffer[i]=NULL;
		for(unsigned i=0;i<4;i++)
		{
			buffer[i]=(char*)malloc(sizeof(char)*bufferLen);
			if(buffer[i]==NULL)
			{
				error="malloc wrong";
				return;
			}
			nowFree.push(buffer[i]);
			memset(buffer[i],0,sizeof(char)*bufferLen);
		}
		now=nowFree.front();
		nowFree.pop();
		page=now;
	}
	~LogSystem()
	{
		std::pair<LogSystem*,char*>* argv=new std::pair<LogSystem*,char*>(this,page);
		worker(argv);
		for(unsigned i=0;i<4;i++)
			if(buffer[i]!=NULL)
				free(buffer[i]);
	}
	void accessLog(const char* text)
	{
		if(text==NULL)
			return;
		if(nowLen+strlen(text)>=bufferLen)
		{
			while(nowFree.empty());
			/* pool.mutexLock(); */
			now=nowFree.front();
			nowFree.pop();
			/* pool.mutexUnlock(); */
			nowLen=0;
			std::pair<LogSystem*,char*>* argv=new std::pair<LogSystem*,char*>(this,page);
			ThreadPool::Task task{worker,argv};
			pool.addTask(task);
			page=now;
		}
		strcat(now,text);
		strcat(now,"\n");
		nowLen+=strlen(text)+1;
	}
	static void recordRequest(const void* text,int )
	{
		static char method[32]={0},askPath[256]={0},buffer[512]={0};
		static LogSystem loger("access.log");
		/* int port=0; */
		sscanf((char*)text,"%32s%256s",method,askPath);
		/* time_t now=time(NULL); */
		/* sprintf(buffer,"%s %s %s %s",ctime(&now),ServerTcpIp::getPeerIp(soc,&port),method,askPath); */
		loger.accessLog(buffer);
	}
	static bool recordFileError(const char* fileName)
	{
		FILE* fp=fopen("wrong.log","r+");
		if(fp==NULL)
			fp=fopen("wrong.log","w+");
		if(fp==NULL)
			return false;
		fseek(fp,0,SEEK_END);
		fprintf(fp,"open file %s wrong\n",fileName);
		fclose(fp);
		return true;
	}
	/* static void httpLog(const void * text,int soc) */
	/* { */
	/* 	DealHttp http; */
	/* 	int port=0; */
	/* 	FILE* fp=fopen("ask.log","a+"); */
	/* 	if(fp==NULL) */
	/* 		fp=fopen("ask.log","w+"); */
	/* 	if(fp==NULL) */
	/* 		return ; */
	/* 	DealHttp::Request req; */
	/* 	http.analysisRequest(req,text); */
	/* 	time_t now=time(NULL); */
	/* 	fprintf(fp,"%s %s %s %s\n",ctime(&now),ServerTcpIp::getPeerIp(soc,&port),req.method.c_str(),req.askPath.c_str()); */
	/* 	fclose(fp); */
	/* 	return ; */
	/* } */
private:
	static void* worker(void* argv)
	{
		std::pair<LogSystem*,char*>& now=*(std::pair<LogSystem*,char*>*)argv;
		FILE* fp=fopen(now.first->fileName,"a+");
		if(fp==NULL)
		{
			now.first->error="open file wrong";
			return NULL;
		}
		fprintf(fp,"%s",now.second);
		fclose(fp);
		memset(now.second,0,now.first->bufferLen);
		/* now.first->pool.mutexLock(); */
		now.first->nowFree.push(now.second);
		/* now.first->pool.mutexUnlock(); */
		delete (std::pair<LogSystem*,char*>*)argv;
		return NULL;
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
