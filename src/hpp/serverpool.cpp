#include "../hpp/cppweb.h"
namespace cppweb{
/*******************************
 * author:chenxuan
 * class:the same as servertcpip but it use threadpool
 * example:see ServerTcpIp
******************************/
class ServerPool:public ServerTcpIp{
private:
	struct Argv{
		ServerPool* pserver;
		void (*func)(ServerPool&,int);
		int soc;
		Argv()
		{
			pserver=NULL;
			soc=-1;
			func=NULL;
		}
	};
private:
	ThreadPool* pool;
	pthread_mutex_t mutex;
	unsigned int threadNum;
	bool isEpoll;
private:
#ifndef _WIN32
	static void sigCliDeal(int )
	{
		while(waitpid(-1, NULL, WNOHANG)>0);
	}
#endif
	static void* worker(void* argc)
	{
		Argv argv=*(Argv*)argc;
		delete (Argv*)argc;
		if(argv.func!=NULL)
			argv.func(*argv.pserver,argv.soc);
		return NULL;
	}
public:
	ServerPool(unsigned short port,unsigned int threadNum=0):ServerTcpIp(port)
	{
		this->threadNum=threadNum;
		if(threadNum>0)
		{	
			pool=new ThreadPool(threadNum);
			if(pool==NULL)
			{
				this->error="malloc wrong";
				return;
			}
		}
		pthread_mutex_init(&mutex,NULL);
		isEpoll=false;
	}
	~ServerPool()
	{
		delete pool;
	   	pthread_mutex_destroy(&mutex);
	}
	inline void mutexLock()
	{
		pthread_mutex_lock(&mutex);
	}
	inline void mutexUnlock()
	{
		pthread_mutex_unlock(&mutex);
	}
	bool mutexTryLock()
	{
		if(0==pthread_mutex_trylock(&mutex))
			return true;
		else
			return false;
	}
	void threadModel(void (*pfunc)(ServerPool&,int))
	{
		if(this->threadNum==0)
		{
			this->error="thread wrong init";
			return;
		}
		while(1)
		{
			sockaddr_in newaddr={0,0,{0},{0}};
			int newClient=acceptSocket(newaddr);
			if(newClient==-1)
				continue;
			Argv* temp=new Argv;
			if(temp==NULL)
			{
				error="malloc wrong";
				return;
			}
			temp->pserver=this;
			temp->func=pfunc;
			temp->soc=newClient;
			ThreadPool::Task task={worker,temp};
			pool->addTask(task);
		}
	}
#ifndef _WIN32
	void forkModel(void* pneed,void (*pfunc)(ServerPool&,int,void*))
	{
		signal(SIGCHLD,sigCliDeal);
		while(1)
		{
			sockaddr_in newaddr={0,0,{0},{0}};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
			if(newClient==-1)
				continue;
			if(fork()==0)
			{
				close(sock);
				pfunc(*this,newClient,pneed);
			}
			close(newClient);
		}
	}
	void forkEpoll(unsigned int senBufChar,unsigned int recBufChar,void (*pfunc)(ServerPool::Thing,int,int,void*,void*,ServerPool&))
	{
		signal(SIGCHLD,sigCliDeal);
		char* pneed=(char*)malloc(sizeof(char)*senBufChar),*pget=(char*)malloc(sizeof(char)*recBufChar);
		if(pneed==NULL||pget==NULL)
		{
			this->error="malloc worng";
			return;
		}
		memset(pneed,0,sizeof(char)*senBufChar);
		memset(pget,0,sizeof(char)*recBufChar);
		while(1)
		{
			int eventNum=epoll_wait(epfd,pevent,512,-1),thing=0;
			for(int i=0;i<eventNum;i++)
			{
				epoll_event temp=pevent[i];
				if(temp.data.fd==sock)
				{
					sockaddr_in newaddr={0,0,{0},{0}};
					int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
					nowEvent.data.fd=newClient;
					nowEvent.events=EPOLLIN;
					epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
					strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
				}
				else
				{
					memset(pget,0,sizeof(char)*recBufChar);
					int getNum=recv(temp.data.fd,(char*)pget,recBufChar,0);
					if(getNum>0)
						thing=2;
					else
					{
						*(char*)pget=0;
						thing=0;
						epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
						close(temp.data.fd);
					}
					if(pfunc!=NULL&&thing==2)
					{
						if(fork()==0)
						{
							close(sock);
							pfunc(CPPSAY,temp.data.fd,getNum,pget,pneed,*this);
							close(temp.data.fd);
							free(pneed);
							free(pget);
							exit(0);
						}
						else
						{
							epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
							close(temp.data.fd);
						}
					}
				}
			}
		}
	}
	void epollThread(void (*pfunc)(ServerPool&,int))
	{
		if(this->threadNum==0)
		{
			this->error="thread wrong init";
			return;
		}
		isEpoll=true;
		int eventNum=epoll_wait(epfd,pevent,512,-1);
		for(int i=0;i<eventNum;i++)
		{
			epoll_event temp=pevent[i];
			if(temp.data.fd==sock)
			{
				sockaddr_in newaddr={0,0,{0},{0}};
				int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
				nowEvent.data.fd=newClient;
				nowEvent.events=EPOLLIN|EPOLLET;
				epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
			}
			else
			{
				if(pfunc!=NULL)
				{
					Argv* argv=new Argv;
					argv->func=pfunc;
					argv->soc=temp.data.fd;
					argv->pserver=this;
					ThreadPool::Task task={worker,argv};
					pool->addTask(task);
				}
			}
		}
		return;
	}
#endif
	inline void threadDeleteSoc(int clisoc)
	{
		closeSocket(clisoc);
#ifndef _WIN32
		if(isEpoll)
			epoll_ctl(epfd,clisoc,EPOLL_CTL_DEL,NULL);
#endif
	}
};
}

