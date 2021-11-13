#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<time.h>
#include<winsock2.h>
#include<pthread.h>
#include<queue>
//#include<openssl/ssl.h>
//#include<openssl/err.h>
using namespace std;
class WSAinit{
public:
	WSAinit()
	{
		WSADATA wsa;//web server api data
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
		{
			printf("wsadata wrong\n");
			exit(0);
		}
	}
	~WSAinit()
	{
		WSACleanup();
	}
};
/********************************
	author:chenxuan
	date:2021/11/3
	funtion:deal json data
*********************************/
class Json{
private:
	char* buffer;
	char word[30];
	const char* text;
	unsigned int nowLen;
	unsigned int maxLen;
public:
	Json()
	{
		buffer=NULL;
		text=NULL;
		nowLen=0;
		maxLen=0;
		memset(this->word,0,sizeof(char)*30);
	}
	Json(const char* jsonText)
	{
		text=jsonText;
		buffer=NULL;
		nowLen=0;
		maxLen=0;
		memset(this->word,0,sizeof(char)*30);		
	}
	~Json()
	{
		if(this->buffer!=NULL)
			free(buffer);
	}
	bool jsonInit(unsigned int bufferLen)
	{
		if(bufferLen<=10)
			return false;
		buffer=(char*)malloc(sizeof(char)*bufferLen);
		memset(buffer,0,sizeof(char)*bufferLen);
		if(buffer==NULL)
			return false;
		this->maxLen=bufferLen;
		strcat(this->buffer,"{");
		this->nowLen+=2;
		return true;
	}
	bool addKeyValue(const char* key,const char* value)
	{
		char temp[200]={0};
		if(nowLen+strlen(key)+strlen(value)>maxLen)
			return false;
		if(strlen(key)+strlen(value)>=180)
			return false;
		int len=sprintf(temp,"\"%s\":\"%s\";",key,value);
		strcat(this->buffer,temp);
		nowLen+=len;
		return true;
	}
	bool addKeyValInt(const char* key,int value)
	{
		char temp[50]={0};
		if(nowLen+50>maxLen)
			return false;
		if(strlen(key)>=45)
			return false;	
		int len=sprintf(temp,"\"%s\":%d;",key,value);
		strcat(this->buffer,temp);
		nowLen+=len;
		return true;	
	}
	bool addKeyValFloat(const char* key,float value,int output)
	{
		char temp[50]={0};
		if(nowLen+50>maxLen)
			return false;
		if(strlen(key)>=45)
			return false;	
		int len=sprintf(temp,"\"%s\":%.*f;",key,output,value);
		strcat(this->buffer,temp);
		nowLen+=len;
		return true;		
	}
	const char* endJson()
	{
		if(nowLen+5>maxLen)
			return NULL;
		strcat(buffer,"}");
		return this->buffer;
	}
	bool jsonToFile(const char* fileName)
	{
		FILE* fp=fopen(fileName,"w+");
		if(fp==NULL)
			return false;
		fprintf(fp,"%s",this->buffer);
		fclose(fp);
		return true;
	}
	const char* operator[](const char* key)
	{
		char* temp=NULL;
		if((temp=strstr((char*)this->text,key))==NULL)
			return NULL;
		temp=strchr(temp,'\"');
		if(temp==NULL)
			return NULL;
		temp=strchr(temp+1,'\"');
		if(temp==NULL)
			return NULL;
		temp++;
		if(strchr(temp,'\"')-temp>30)
			return NULL;
		memset(this->word,0,sizeof(char)*30);
		for(unsigned int i=0;*temp!='\"';i++,temp++)
			word[i]=*temp;
		return word;
	}
	float getValueFloat(const char* key,bool& flag)
	{
		float value=0;
		char* temp=strstr((char*)text,key);
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		temp=strchr(temp,'\"');
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		temp=strchr(temp+1,':');
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		if(sscanf(temp+1,"%f",&value)<=0)
		{
			flag=true;
			return -1;
		}
		flag=true;
		return value;
	}
	int getValueInt(const char* key,bool& flag)
	{
		int value=0;
		char* temp=strstr((char*)text,key);
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		temp=strchr(temp,'\"');
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		temp=strchr(temp+1,':');
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		if(sscanf(temp+1,"%d",&value)<=0)
		{
			flag=true;
			return -1;
		}
		flag=true;
		return value;
	}
};
/********************************
	author:chenxuan
	date:2021/11/7
	funtion:webtoken as jwt
*********************************/
class WebToken{
private:
	char* backString;
	char err[30];
public:
	WebToken()
	{
		backString=NULL;
		memset(err,0,sizeof(char)*30);
		sprintf(err,"no error");
	}
	const char* createToken(const char* key,const char* encryption,char* getString,unsigned int stringLen,unsigned int liveSecond)
	{
		int temp=0;
		if(key==NULL||encryption==NULL||getString==NULL||stringLen<sizeof(char)*strlen(encryption)+30)
		{
			sprintf(err,"input wrong");
			return NULL;
		}	
		if(backString!=NULL)
			free(backString);
		backString=(char*)malloc(sizeof(char)*strlen(encryption)+30);
		memset(backString,0,sizeof(char)*strlen(encryption)+30);
		if(backString==NULL)
		{
			sprintf(err,"malloc wrong");
			return NULL;
		}
		for(unsigned int i=0;i<strlen(key);i++)
			temp+=key[i];
		int end=time(NULL)+liveSecond+temp;
		temp=temp%4;
		for(unsigned int i=0;i<strlen(encryption);i++)
		{
			backString[i]=encryption[i]-temp;
			if(backString[i]==94)
				backString[i]=92;
		}
		char tempString[30]={0},endString[30]={0};
		sprintf(endString,"%d",end);
		for(unsigned int i=0;i<strlen(endString);i++)
			endString[i]+=50;
		sprintf(tempString,"&%s&%c",endString,encryption[0]);
		strcat(backString,tempString);
		strcpy(getString,backString);
		return getString;
	}
	const char* decryptToken(const char* key,const char* token,char* buffer,unsigned int bufferLen)
	{
		char* temp=strchr((char*)token,'&');
		if(temp==NULL||key==NULL||token==NULL||buffer==NULL||bufferLen<strlen(token))
		{
			sprintf(err,"input wrong");
			return NULL;
		}
		if(backString!=NULL)
			free(backString);
		backString=(char*)malloc(sizeof(char)*strlen(token));
		memset(backString,0,sizeof(char)*strlen(token));
		char endString[20]={0};
		if(sscanf(temp+1,"%[^&]",endString)<=0)
		{
			sprintf(err,"get time wrong");
			return NULL;
		}
		int keyTemp=0,end=0;
		for(unsigned int i=0;i<strlen(endString);i++)
			endString[i]-=50;
		sscanf(endString,"%d",&end);
		for(unsigned int i=0;i<strlen(key);i++)
			keyTemp+=key[i];
		end-=keyTemp;
		if(end-time(NULL)<=0)
		{
			sprintf(err,"over time");
			return NULL;
		}
		keyTemp=keyTemp%4;
		unsigned int i=0;
		for(i=0;i+token<temp;i++)
			if(token[i]!=92)
				backString[i]=token[i]+keyTemp;
			else
				backString[i]=97;
		backString[i+1]=0;
		if(backString[0]!=token[strlen(token)-1])
		{
			sprintf(err,"key wrong");
			return NULL;
		}
		strcpy(buffer,backString);
		return buffer;
	}
	inline const char* LastError()
	{
		return err;
	}
};
/********************************
	author:chenxuan
	date:2021/8/10
	funtion:class thread pool
*********************************/
class ThreadPool{
public://a struct for you to add task
	struct Task{
		void* (*ptask)(void*);
		void* arg;
	};
private:
	queue<Task> thingWork;//a queue for struct task
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
	static void* worker(void* arg)//the worker for user 
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
	static void* manager(void* arg)//manager for user
	{
		return NULL;
	}
public:
	ThreadPool(unsigned int threadNum=10)//create threads
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
	~ThreadPool()//destory pool
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
	void threadExit()// a no use funtion
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
	void addTask(Task task)//by this you can add task
	{
		if(isContinue==false)
			return;
		pthread_mutex_lock(&this->lockPoll);
		this->thingWork.push(task);
		pthread_mutex_unlock(&this->lockPoll);
		pthread_cond_signal(&this->condition);
	}
	void endPool()//user delete the pool
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
	void getBusyAndTask(unsigned int* pthread,unsigned int* ptask)//get busy live and task num
	{
		pthread_mutex_lock(&lockBusy);
		*pthread=busyThread;
		pthread_mutex_unlock(&lockBusy);
		pthread_mutex_lock(&lockPoll);
		*ptask=thingWork.size();
		pthread_mutex_unlock(&lockPoll);
	}
	inline void mutexLock()//user to lock ctrl
	{
		pthread_mutex_lock(&this->lockTask);
	}
	inline void mutexUnlock()//user to lock ctrl
	{
		pthread_mutex_unlock(&this->lockTask);
	}
	static pthread_t createPthread(void* arg,void* (*pfunc)(void*))//create a thread 
	{
		pthread_t thread=0;
		pthread_create(&thread,NULL,pfunc,arg);
		return thread;
	}
	static inline void waitPthread(pthread_t thread,void** preturn=NULL)//wait the thread end
	{
		pthread_join(thread,preturn);
	}
	static void createDetachPthread(void* arg,void* (*pfunc)(void*))//create a ddetach thread
	{
		pthread_t thread=0;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_create(&thread,&attr,pfunc,arg);
	}
};
class ServerTcpIp{
protected:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now; 
	char* hostip;//host IP 
	char* hostname;//host name
	WSADATA wsa;//apply verson of windows;
	SOCKET sock;//file descriptor of host;
	SOCKET sockC;//file descriptor to sign every client;
	SOCKADDR_IN addr;//IPv4 of host;
	SOCKADDR_IN client;//IPv4 of client;
	fd_set  fdClients;//file descriptor
protected:
    int* pfdn;//pointer if file descriptor
    int fdNumNow;//num of fd now
    int fdMax;//fd max num
    bool addFd(int addsoc)
    {
        bool flag=false;
        for(int i=0;i<fdNumNow;i++)
        {
            if(pfdn[i]==0)
            {
                pfdn[i]=addsoc;
                flag=true;
                break;
            }
        }
        if(flag==false)
        {
            if(fdNumNow>=fdMax)
            {
                pfdn=(int*)realloc(pfdn,sizeof(int)*(fdMax+32));
                if(pfdn==NULL)
                    return false;
                fdMax+=31;
            }
            pfdn[fdNumNow]=addsoc;
            fdNumNow++;
        }
        return true;
    }
    bool deleteFd(int clisoc)
    {
        for(int i=0;i<fdNumNow;i++)
        {
            if(pfdn[i]==clisoc)
            {
                pfdn[i]=0;
                return true;
            }
        }
        return false;
    } 
public:
	ServerTcpIp(unsigned short port=5200,int wait=5)
	{
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
			throw NULL;
		sock=socket(AF_INET,SOCK_STREAM,0);//AF=addr family internet
		addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//inaddr_any
		addr.sin_family=AF_INET;//af_intt IPv4
		addr.sin_port=htons(port);//host to net short
		sizeAddr=sizeof(sockaddr);
		backwait=wait;
		numClient=0;
		fdNumNow=0;
		fdMax=100;
		pfdn=(int*)malloc(sizeof(int)*100);
		memset(pfdn,0,sizeof(int)*100);
		hostip=(char*)malloc(sizeof(char)*20);
		memset(hostip,0,sizeof(char)*20);
		hostname=(char*)malloc(sizeof(char)*30);
		memset(hostname,0,sizeof(char)*30);
		FD_ZERO(&fdClients);//clean fdClients;
	}
	~ServerTcpIp()//clean server
	{
		closesocket(sock);
		closesocket(sockC);
		free(pfdn);
		free(hostip);
		free(hostname);
		WSACleanup();
	}
	bool bondhost()//bond myself
	{
		if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
			return false;
		return true;
	}
	bool setlisten()//set listem to accept
	{
		if(listen(sock,backwait)==SOCKET_ERROR)
			return false;
		FD_SET(sock,&fdClients);
		return true;
	}
	unsigned int acceptClient()//wait until success
	{
		sockC=accept(sock,(sockaddr*)&client,&sizeAddr);
		return sockC;
	}
	bool acceptClients(unsigned int* psock)//model two
	{
		*psock=accept(sock,(sockaddr*)&client,&sizeAddr);
		return this->addFd(*psock);
	}
	inline int receiveOne(void* pget,int len)//model one
	{
		return recv(sockC,(char*)pget,len,0);
	}
	inline int receiveSocket(unsigned int sock,void* prec,int len)//model two
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendClientOne(const void* psend,int len)//model one
	{
		return send(sockC,(char*)psend,len,0);
	}
	inline int sendSocket(unsigned int sock ,const void* psend,int len)//model two
	{
		return send(sock,(char*)psend,len,0);
	}
	void sendEverySocket(const void* psen,int len)//model two
	{
		for(int i=0;i<fdNumNow;i++)
			if(pfdn[i]!=0)
			    send(pfdn[i],(char*)psen,len,0);
	}
	bool epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIp& ))
	{//0 out,1 in,2 say
		fd_set temp=fdClients;
		int sign=select(0,&temp,NULL,NULL,NULL);
		if(sign>0)
		{
			for(int i=0;i<(int)fdClients.fd_count;i++)
			{
				if(FD_ISSET(fdClients.fd_array[i],&temp))
				{
					if(fdClients.fd_array[i]==sock)
					{
						if(fdClients.fd_count<FD_SETSIZE)
						{
							SOCKADDR_IN newaddr={0};
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							*pthing=1;
							*pnum=newClient;
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(pfunc!=NULL)
							{
								if(pfunc(*pthing,*pnum,0,pget,pneed,*this))
									return false;
							}
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						*pnum=fdClients.fd_array[i];
						if(sRec>0)
							*pthing=2;
						if(sRec<=0)
						{
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);
							*pthing=0;
						}
						if(pfunc!=NULL)
						{
							if(pfunc(*pthing,*pnum,sRec,pget,pneed,*this))
								return false;
						}
					}
				}
			}
		}
		else
			return false;
		return true;
	}	
	bool selectModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIp& ))
	{//0 out,1 in,2 say
		fd_set temp=fdClients;
		int sign=select(0,&temp,NULL,NULL,NULL);
		if(sign>0)
		{
			for(int i=0;i<(int)fdClients.fd_count;i++)
			{
				if(FD_ISSET(fdClients.fd_array[i],&temp))
				{
					if(fdClients.fd_array[i]==sock)
					{
						if(fdClients.fd_count<FD_SETSIZE)
						{
							SOCKADDR_IN newaddr={0};
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							*pthing=1;
							*pnum=newClient;
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(pfunc!=NULL)
							{
								if(pfunc(*pthing,*pnum,0,pget,pneed,*this))
									return false;
							}
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						*pnum=fdClients.fd_array[i];
						if(sRec>0)
							*pthing=2;
						if(sRec<=0)
						{
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);
							*pthing=0;
						}
						if(pfunc!=NULL)
						{
							if(pfunc(*pthing,*pnum,sRec,pget,pneed,*this))
								return false;
						}
					}
				}
			}
		}
		else
			return false;
		return true;
	}
//	bool selectSendEveryone(void* psend,int len)
//	{
//		for(unsigned int i=0;i<fdClients.fd_count;i++)
//			if(fdClients.fd_array[i]!=0)
//				send(fdClients.fd_array[i],(char*)psend,len,0);
//		return true;
//	}
//	bool updateSocketSelect(SOCKET* psocket,int* pcount)
//	{
//		if(fdClients.fd_count!=0)
//			*pcount=fdClients.fd_count;
//		else
//			return false;
//		for(unsigned int i=0;i<fdClients.fd_count;i++)
//			psocket[i]=fdClients.fd_array[i];
//		return true;
//	}
//	bool sendSocketSelect(SOCKET toClient,const void* psend,int len)
//	{
//		for(unsigned int i=0;i<fdClients.fd_count;i++)
//		{
//			if(toClient==fdClients.fd_array[i])
//			{
//				send(fdClients.fd_array[i],(char*)psend,len,0);
//				return true;
//			}
//		}
//		return false;
//	}
//	SOCKET findSocketSelect(int i)
//	{
//		if(fdClients.fd_array[i]!=0)
//			return fdClients.fd_array[i];
//		else
//			return -1;
//	}
    bool disconnectSocket(SOCKET clisock)
    {
        for(unsigned int i=0;i<fdClients.fd_count;i++)
            if(fdClients.fd_array[i]==clisock)
                FD_CLR(fdClients.fd_array[i],&fdClients);
        closesocket(clisock);
        return this->deleteFd(clisock);
    }
	bool updateSocket(SOCKET* array,int* pcount,int arrayLen)
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow&&i<arrayLen;i++)
			array[i]=pfdn[i];
		return true;		
	}
	bool findSocket(int cliSoc)
	{
		for(int i=0;i<fdNumNow;i++)
			if(pfdn[i]==cliSoc)
				return true;
		return false;
	}
	char* getHostIp()
	{
		char name[30]={0};
		gethostname(name,30);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.S_un.S_addr,p,phost->h_length);
		memset(hostip,0,sizeof(char)*20);
		memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return hostip;
	}
	char* getHostName()
	{
		char name[30]={0};
		gethostname(name,30);
		memcpy(hostname,name,30);
		return hostname;
	}
	char* getPeerIp(SOCKET cliSoc,int* pcliPort)
	{
		SOCKADDR_IN cliAddr={0};
		int len=sizeof(cliAddr);
		if(-1==getpeername(cliSoc,(SOCKADDR*)&cliAddr,&len))
			return NULL;
		*pcliPort=cliAddr.sin_port;
		return inet_ntoa(cliAddr.sin_addr); 
	}
};
class ServerTcpIpThreadPool{
private:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now; 
	char* hostip;//host IP 
	char* hostname;//host name
	WSADATA wsa;//apply verson of windows;
	SOCKET sock;//file descriptor of host;
	SOCKET sockC;//file descriptor to sign every client;
	SOCKADDR_IN addr;//IPv4 of host;
	SOCKADDR_IN client;//IPv4 of client;
	fd_set  fdClients;//file descriptor
	ThreadPool* pool;
protected:
    int* pfdn;//pointer if file descriptor
    int fdNumNow;//num of fd now
    int fdMax;//fd max num
    bool addFd(int addsoc)
    {
        bool flag=false;
        for(int i=0;i<fdNumNow;i++)
        {
            if(pfdn[i]==0)
            {
                pfdn[i]=addsoc;
                flag=true;
                break;
            }
        }
        if(flag==false)
        {
            if(fdNumNow>=fdMax)
            {
                pfdn=(int*)realloc(pfdn,sizeof(int)*(fdMax+32));
                if(pfdn==NULL)
                    return false;
                fdMax+=31;
            }
            pfdn[fdNumNow]=addsoc;
            fdNumNow++;
        }
        return true;
    }
    bool deleteFd(int clisoc)
    {
        for(int i=0;i<fdNumNow;i++)
        {
            if(pfdn[i]==clisoc)
            {
                pfdn[i]=0;
                return true;
            }
        }
        return false;
    } 
public:
	struct ArgvSer{
		ServerTcpIpThreadPool& server;
		int soc;
		void* pneed;
	};
	struct ArgvSerEpoll{
		ServerTcpIpThreadPool& server;
		int soc;
		int thing;
		int len;
		void* pneed;
		void* pget;	
	};
public:
	ServerTcpIpThreadPool(unsigned short port=5200,int wait=5,int threadNum=0)
	{
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
			throw NULL;
		sock=socket(AF_INET,SOCK_STREAM,0);//AF=addr family internet
		addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//inaddr_any
		addr.sin_family=AF_INET;//af_intt IPv4
		addr.sin_port=htons(port);//host to net short
		sizeAddr=sizeof(sockaddr);
		backwait=wait;
		numClient=0;
		fdNumNow=0;
		fdMax=100;
		pfdn=(int*)malloc(sizeof(int)*100);
		memset(pfdn,0,sizeof(int)*100);
		hostip=(char*)malloc(sizeof(char)*20);
		memset(hostip,0,sizeof(char)*20);
		hostname=(char*)malloc(sizeof(char)*30);
		memset(hostname,0,sizeof(char)*30);
		FD_ZERO(&fdClients);//clean fdClients;
		if(threadNum>0)
		{	
			pool=new ThreadPool(threadNum);
			if(pool==NULL)
				throw NULL;
		}
	}
	~ServerTcpIpThreadPool()//clean server
	{
		closesocket(sock);
		closesocket(sockC);
		free(pfdn);
		free(hostip);
		free(hostname);
		WSACleanup();
	}
	bool bondhost()//bond myself
	{
		if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
			return false;
		return true;
	}
	bool setlisten()//set listem to accept
	{
		if(listen(sock,backwait)==SOCKET_ERROR)
			return false;
		FD_SET(sock,&fdClients);
		return true;
	}
	unsigned int acceptClient()//wait until success
	{
		sockC=accept(sock,(sockaddr*)&client,&sizeAddr);
		return sockC;
	}
	bool acceptClients(unsigned int* psock)//model two
	{
		*psock=accept(sock,(sockaddr*)&client,&sizeAddr);
		return this->addFd(*psock);
	}
	inline int receiveOne(void* pget,int len)//model one
	{
		return recv(sockC,(char*)pget,len,0);
	}
	inline int receiveSocket(unsigned int sock,void* prec,int len)//model two
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendClientOne(const void* psend,int len)//model one
	{
		return send(sockC,(char*)psend,len,0);
	}
	inline int sendSocket(unsigned int sock ,const void* psend,int len)//model two
	{
		return send(sock,(char*)psend,len,0);
	}
	void sendEverySocket(const void* psen,int len)//model two
	{
		for(int i=0;i<fdNumNow;i++)
			if(pfdn[i]!=0)
			    send(pfdn[i],(char*)psen,len,0);
	}
	bool epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIpThreadPool& ))
	{//0 out,1 in,2 say
		fd_set temp=fdClients;
		int sign=select(0,&temp,NULL,NULL,NULL);
		if(sign>0)
		{
			for(int i=0;i<(int)fdClients.fd_count;i++)
			{
				if(FD_ISSET(fdClients.fd_array[i],&temp))
				{
					if(fdClients.fd_array[i]==sock)
					{
						if(fdClients.fd_count<FD_SETSIZE)
						{
							SOCKADDR_IN newaddr={0};
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							*pthing=1;
							*pnum=newClient;
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(pfunc!=NULL)
							{
								if(pfunc(*pthing,*pnum,0,pget,pneed,*this))
									return false;
							}
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						*pnum=fdClients.fd_array[i];
						if(sRec>0)
							*pthing=2;
						if(sRec<=0)
						{
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);
							*pthing=0;
						}
						if(pfunc!=NULL)
						{
							if(pfunc(*pthing,*pnum,sRec,pget,pneed,*this))
								return false;
						}
					}
				}
			}
		}
		else
			return false;
		return true;
	}	
	bool selectModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIpThreadPool& ))
	{//0 out,1 in,2 say
		fd_set temp=fdClients;
		int sign=select(0,&temp,NULL,NULL,NULL);
		if(sign>0)
		{
			for(int i=0;i<(int)fdClients.fd_count;i++)
			{
				if(FD_ISSET(fdClients.fd_array[i],&temp))
				{
					if(fdClients.fd_array[i]==sock)
					{
						if(fdClients.fd_count<FD_SETSIZE)
						{
							SOCKADDR_IN newaddr={0};
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							*pthing=1;
							*pnum=newClient;
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(pfunc!=NULL)
							{
								if(pfunc(*pthing,*pnum,0,pget,pneed,*this))
									return false;
							}
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						*pnum=fdClients.fd_array[i];
						if(sRec>0)
							*pthing=2;
						if(sRec<=0)
						{
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);
							*pthing=0;
						}
						if(pfunc!=NULL)
						{
							if(pfunc(*pthing,*pnum,sRec,pget,pneed,*this))
								return false;
						}
					}
				}
			}
		}
		else
			return false;
		return true;
	}
    void threadModel(void* pneed,void* (*pfunc)(void*))
    {
		ServerTcpIpThreadPool::ArgvSer argv={*this,0,pneed};
		ThreadPool::Task task={pfunc,&argv};
    	while(1)
    	{
			sockaddr_in newaddr={0};
			int newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
			if(newClient==-1)
				continue;
			this->addFd(newClient);
			argv.soc=newClient;
			task.arg=&argv;
			pool->addTask(task);
		}
	}
	bool selectSendEveryone(void* psend,int len)
	{
		for(unsigned int i=0;i<fdClients.fd_count;i++)
			if(fdClients.fd_array[i]!=0)
				send(fdClients.fd_array[i],(char*)psend,len,0);
		return true;
	}
	bool updateSocketSelect(SOCKET* psocket,int* pcount)
	{
		if(fdClients.fd_count!=0)
			*pcount=fdClients.fd_count;
		else
			return false;
		for(unsigned int i=0;i<fdClients.fd_count;i++)
			psocket[i]=fdClients.fd_array[i];
		return true;
	}
	bool sendSocketSelect(SOCKET toClient,const void* psend,int len)
	{
		for(unsigned int i=0;i<fdClients.fd_count;i++)
		{
			if(toClient==fdClients.fd_array[i])
			{
				send(fdClients.fd_array[i],(char*)psend,len,0);
				return true;
			}
		}
		return false;
	}
	SOCKET findSocketSelect(int i)
	{
		if(fdClients.fd_array[i]!=0)
			return fdClients.fd_array[i];
		else
			return -1;
	}
    bool disconnectSocket(SOCKET clisock)
    {
        for(unsigned int i=0;i<fdClients.fd_count;i++)
            if(fdClients.fd_array[i]==clisock)
                FD_CLR(fdClients.fd_array[i],&fdClients);
        closesocket(clisock);
        return this->deleteFd(clisock);
    }
	bool updateSocket(SOCKET* array,int* pcount,int arrayLen)
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow&&i<arrayLen;i++)
			array[i]=pfdn[i];
		return true;		
	}
	bool findSocket(int cliSoc)
	{
		for(int i=0;i<fdNumNow;i++)
			if(pfdn[i]==cliSoc)
				return true;
		return false;
	}
	char* getHostIp()
	{
		char name[30]={0};
		gethostname(name,30);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.S_un.S_addr,p,phost->h_length);
		memset(hostip,0,sizeof(char)*20);
		memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return hostip;
	}
	char* getHostName()
	{
		char name[30]={0};
		gethostname(name,30);
		memcpy(hostname,name,30);
		return hostname;
	}
	char* getPeerIp(SOCKET cliSoc,int* pcliPort)
	{
		SOCKADDR_IN cliAddr={0};
		int len=sizeof(cliAddr);
		if(-1==getpeername(cliSoc,(SOCKADDR*)&cliAddr,&len))
			return NULL;
		*pcliPort=cliAddr.sin_port;
		return inet_ntoa(cliAddr.sin_addr); 
	}
};//D:/Dev-Cpp/MinGW64/x86_64-w64-mingw32/lib/libws2_32.a
class ClientTcpIp{
private:
	WSADATA wsa;//apply for api
	SOCKET sock;//myself
	SOCKADDR_IN addrC;//server information
	char ip[20];//server Ip
	char sen[200];//what you send
	char rec[200];//what you get
	char* hostip;//host ip
	char* hostname;//host name
//	SSL* ssl;
//	SSL_CTX* ctx;
public:
	ClientTcpIp(const char* hostIp,int port=5200)
	{
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
			exit(0);
		memset(ip,0,20);
		if(hostIp!=NULL)
			strcpy(ip,hostIp);
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(hostIp!=NULL)
			addrC.sin_addr.S_un.S_addr=inet_addr(ip);
		addrC.sin_family=AF_INET;
		addrC.sin_port=htons(port);
		hostip=(char*)malloc(sizeof(char)*20);
		memset(hostip,0,sizeof(char)*20);
		hostname=(char*)malloc(sizeof(char)*30);
		memset(hostname,0,sizeof(char)*30);
//		ssl=NULL;
//		ctx=NULL;
	}
	~ClientTcpIp()
	{
//		if(ssl!=NULL)
//		{
//			SSL_shutdown(ssl);
//			SSL_free(ssl);	
//		}
//		if(ctx!=NULL)
//			SSL_CTX_free(ctx);
		free(hostip);
		free(hostname);
		closesocket(sock);
		WSACleanup();
	}
	void addHostIp(const char* ip)
	{
		if(ip==NULL)
			return;
		strcpy(this->ip,ip);
		addrC.sin_addr.S_un.S_addr=inet_addr(ip);
	}
	bool tryConnect()
	{
		if(connect(sock,(SOCKADDR*)&addrC,sizeof(sockaddr))==SOCKET_ERROR)
			return false;
		return true;
	}
	inline int receiveHost(void* prec,int len)
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendHost(const void* psend,int len)
	{
		return send(sock,(char*)psend,len,0);
	}
	bool disconnectHost()
	{
		closesocket(sock);
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(sock<=0)
			return false;
		return true;
	}
	char* getSelfIp()
	{
		char name[30]={0};
		gethostname(name,30);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.S_un.S_addr,p,phost->h_length);
		memset(hostip,0,sizeof(char)*20);
		memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return hostip;
	}
	char* getSelfName()
	{
		char name[30]={0};
		gethostname(name,30);
		memcpy(hostname,name,30);
		return hostname;
	}
	static bool getDnsIp(const char* name,char* ip,unsigned int ipMaxLen)
	{
		WSADATA wsa;
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
			return false;
		hostent* phost=gethostbyname(name);
		if(phost==NULL)
			return false;
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.S_un.S_addr,p,phost->h_length);
		if(strlen(inet_ntoa(addr))<=ipMaxLen)
			return false;
		strcpy(ip,inet_ntoa(addr));
		return true;
	} 
//	bool sslInit()
//	{
//		const SSL_METHOD* meth=SSLv23_client_method();
//		if(meth==NULL)
//			return false;
//		ctx=SSL_CTX_new(meth);
//		if(ctx==NULL)
//			return false;
//		ssl=SSL_new(ctx);
//		if(NULL==ssl)
//			return false;
//		SSL_set_fd(ssl,sock);
//		int ret=SSL_connect(ssl);
//		if(ret==-1)
//			return false;
//		return true;
//	}
//	inline int sendhostSSL(const void* psen,int len)
//	{
//		return SSL_write(ssl,psen,len);
//	}
//	inline int receiveHostSSL(void* buffer,int len)
//	{
//		return SSL_read(ssl,buffer,len);
//	}
};
/********************************
	author:chenxuan
	date:2021/8/10
	funtion:class to deal easy http ask(get)
*********************************/
class DealHttp{
private:
	char ask[256];
	char* pfind;
public:
	enum FileKind{
		UNKNOWN=0,HTML=1,EXE=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP7=7,JSON=8,
	};
public:
	DealHttp()
	{
		for(int i=0;i<256;i++)
			ask[i]=0;
		pfind=NULL;
	}
	bool cutLineAsk(char* pask,const char* pcutIn)
	{
		char* ptemp=strstr(pask,pcutIn);
		if(ptemp==NULL)
			return false;
		while(*(ptemp++)!='\n');
		*ptemp=0;
		return true;
	}
	const char* analysisHttpAsk(void* pask,const char* pneed="GET",int needLen=3)
	{
		pfind=strstr((char*)pask,pneed);
		if(pfind==NULL)
			return NULL;
		return this->findBackString(pfind,needLen,ask,256);
	}
	inline char* findFirst(void* pask,const char* ptofind)
	{
		return strstr((char*)pask,ptofind);
	}
	char* findBackString(char* local,int len,char* word,int maxWordLen)
	{
		int i=0;
		char* ptemp=local+len+1;
		char* pend=NULL;
		while(1)
			if((*ptemp>47&&*ptemp<58)||(*ptemp>96&&*ptemp<123)||(*ptemp>64&&*ptemp<91)||*ptemp==95)
				break;
			else
				ptemp++;
		pend=ptemp;
		while(1)
			if((*pend>90&&*pend<97&&*pend!=95)||(*pend<48&&*pend!=46&&*pend!=47&&*pend!=45&&*pend!=43)||*pend>122||*pend==63)
				break;
			else
				pend++;
		for(char* pi=ptemp;pi<pend&&i<maxWordLen;pi++)
			word[i++]=*pi;
		word[i]=0;
		return word;
	}
	void createTop(FileKind kind,char* ptop,int* topLen,int fileLen)//1:http 2:down 3:pic
	{
		switch (kind)
		{
			case UNKNOWN:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case HTML:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:text/html\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case EXE:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:application/octet-stream\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case IMAGE:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:image\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case NOFOUND:
				*topLen=sprintf(ptop,"HTTP/1.1 404 Not Found\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length:%d\r\n\r\n"
				"404 no found",(int)strlen("404 no found"));
				break;
			case CSS:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:text/css\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case JS:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:text/javascript\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case ZIP7:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:application/x-7z-compressed\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case JSON:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:application/json\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
		}
	}
	bool createSendMsg(FileKind kind,char* buffer,const char* pfile,int* plong)
	{
		int temp=0;
		int len=0,noUse=0;
		if(kind==NOFOUND)
		{
			this->createTop(kind,buffer,&temp,len);
			*plong=len+temp+1;
			return true;
		}
		len=this->getFileLen(pfile);
		if(len==0)
			return false;
		this->createTop(kind,buffer,&temp,len);
		this->findFileMsg(pfile,&noUse,buffer+temp);
		*plong=len+temp+1;
		return true;
	}
	char* findFileMsg(const char* pname,int* plen,char* buffer)
	{
		FILE* fp=fopen(pname,"rb+");
		int flen=0,i=0;
		if(fp==NULL)
			return NULL;
		fseek(fp,0,SEEK_END);
		flen=ftell(fp);
		fseek(fp,0,SEEK_SET);
		for(i=0;i<flen;i++)
			buffer[i]=fgetc(fp);
		buffer[i]=0;
		*plen=flen;
		fclose(fp);
		return buffer;
	}
	int getFileLen(const char* pname)
	{
		FILE* fp=fopen(pname,"r+");
		int len=0;
		if(fp==NULL)
			return 0;
		fseek(fp,0,SEEK_END);
		len=ftell(fp);
		fclose(fp);
		return len;
	}
	int autoAnalysisGet(const char* pask,char* psend,const char* pfirstFile,int* plen)
	{
		if(NULL==this->analysisHttpAsk((void*)pask))
	        return 0;
	    if(strcmp(ask,"HTTP/1.1")==0||strcmp(ask,"HTTP/1.0")==0)
	    {
	        if(false==this->createSendMsg(HTML,psend,pfirstFile,plen))
	        {
	            if(false==this->createSendMsg(NOFOUND,psend,pfirstFile,plen))
	                return 0;
	            else 
	                return 2;
	        }
	        else
	        	return 1;
	    }
	    else if(strstr(ask,".html"))
	    {
	        if(false==this->createSendMsg(HTML,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;
	    }
	    else if(strstr(ask,".exe"))
	    {
	        if(false==this->createSendMsg(EXE,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;	        
	    }
	    else if(strstr(ask,".png")||strstr(ask,".PNG")||strstr(ask,".jpg")||strstr(ask,".jpeg"))
	    {
	        if(false==this->createSendMsg(IMAGE,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;	                
	    }
	    else if(strstr(ask,".css"))
	    {
	        if(false==this->createSendMsg(CSS,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;	                
	    }
	    else if(strstr(ask,".js"))
	    {
	        if(false==this->createSendMsg(JS,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;
	    }
		else if(strstr(ask,".json"))
	    {
	        if(false==this->createSendMsg(JSON,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;
	    }
	    else 
	    {
	        if(false==this->createSendMsg(UNKNOWN,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;
		}
	    return 1;
	}
	const char* getKeyValue(const void* message,const char* key,char* value,int maxValueLen)
	{
		char* temp=strstr((char*)message,key);
		if(temp==NULL)
			return NULL;
		return this->findBackString(temp,strlen(key),value,maxValueLen);
	}
	const char* getKeyLine(const void* message,const char* key,char* line,int maxLineLen)
	{
		int i=0;
		char* ptemp=strstr((char*)message,key);
		if(ptemp==NULL)
			return NULL;
		ptemp+=strlen(key);
		while(*(ptemp++)!='\r'&&i<maxLineLen)
			line[i++]=*ptemp;
		line[i-1]=0;
		return line;
	}
	const char* getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen)
	{
		char* temp=strstr((char*)message,askWay);
		if(temp==NULL)
			return NULL;
		sscanf(temp+strlen(askWay)+1,"%s",buffer);
		return buffer;
	}
	const char* getRouteValue(const void* routeMsg,const char* key,char* value,unsigned int valueLen)
	{
		char* temp=strstr((char*)routeMsg,key);
		if(temp==NULL)
			return NULL;
		return this->findBackString(temp,strlen(key),value,valueLen);
	}
	const char* getWildUrl(const void* getText,const char* route,char* buffer,int maxLen)
	{
		char* temp=strstr((char*)getText,route);
		if(temp==NULL)
			return NULL;
		temp+=strlen(route);
		sscanf(temp,"%s",buffer);
		return buffer;
	}
	int getRecFile(const void* message,char* fileName,int nameLen,char* buffer,int bufferLen)
	{
		char tempLen[20]={0},*end=NULL,*top=NULL;
		int result=0;
		if(NULL==this->getKeyLine(message,"boundary",buffer,bufferLen))
			return 0;
		if(NULL==this->getKeyValue(message,"filename",fileName,nameLen))
			return 0;
		if(NULL==this->getKeyValue(message,"Content-Length",tempLen,20))
			return 0;
		if(0>=sscanf(tempLen,"%d",&result))
			return 0;
		if((top=strstr((char*)message,buffer))==NULL)
			return 0;
		if((top=strstr(top+strlen(buffer),buffer))==NULL)
			return 0;
		if((end=strstr(top+strlen(buffer),buffer))==NULL)
			return 0;
		if((top=strstr(top,"\r\n\r\n"))==NULL)
			return 0;
		if(end-top>bufferLen)
			return 0;
		top+=4;
		end-=2;
		unsigned int i=0;
		for(i=0;top!=end;i++,top++)
			buffer[i]=*top;
		buffer[i+1]=0;
		return result;
	}
	static void dealUrl(const char* url,char* urlTop,char* urlEnd)
	{
		const char* ptemp=NULL;
		int len=0;
		if((ptemp=strstr(url,"http://"))==NULL)
		{
			if(strstr(url,"https://")!=NULL)
			{
				sscanf(url+8,"%[^/]",urlTop);
				len=strlen(urlTop);
				sscanf(url+len+8,"%s",urlEnd);
				return;
			}
			else
			{
				sscanf(url,"%[^/]",urlTop);
				len=strlen(urlTop);
				sscanf(url+len,"%s",urlEnd);
				return;
			}
		}
		else
		{
			sscanf(url+7,"%[^/]",urlTop);
			len=strlen(urlTop);
			sscanf(url+len+7,"%s",urlEnd);
		}
	}
};
/********************************
	author:chenxuan
	date:2021/11/7
	funtion:version 2.0
*********************************/
class HttpServer:private ServerTcpIp{
public:
	enum RouteType{
		ONEWAY,WILD,
	};
	enum AskType{
		GET,POST,ALL,
	};
	struct RouteFuntion{
		AskType ask;
		RouteType type;
		char route[100];
		void (*pfunc)(DealHttp&,HttpServer&,int num,void* sen,int&);
	};
private:
	RouteFuntion* array;
	void* getText;
	char* error;
	unsigned int max;
	unsigned int now;
	bool isDebug;
	void (*clientIn)(HttpServer&,int num,void* ip,int port);
	void (*clientOut)(HttpServer&,int num,void* ip,int port);
public:
	HttpServer(unsigned port,bool debug=false):ServerTcpIp(port)
	{
		getText=NULL;
		array=NULL;
		error=NULL;
		array=(RouteFuntion*)malloc(sizeof(RouteFuntion)*20);
		if(array==NULL)
			throw NULL;
		error=(char*)malloc(sizeof(char)*100);
		if(error==NULL)
			throw NULL;
		now=0;
		max=20;
		isDebug=debug;
		clientIn=NULL;
		clientOut=NULL;
	}
	~HttpServer()
	{
		if(array!=NULL)
			free(array);
		if(error!=NULL)
			free(error);
	}
	bool routeHandle(AskType ask,RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
	{
		if(strlen(route)>100)
			return false;
		if(max-now<=2)
		{
			array=(RouteFuntion*)realloc(array,sizeof(RouteFuntion)*(now+10));
			if(array=NULL)
				return false;
			max+=10;
		}
		array[now].type=type;
		array[now].ask=ask;
		strcpy(array[now].route,route);
		array[now].pfunc=pfunc;
		now++;
		return true;
	}
	bool clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
	{
		if(clientIn!=NULL)
			return false;
		clientIn=pfunc;
	}
	bool clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
	{
		if(clientOut!=NULL)
			return false;
		clientOut=pfunc;
	}
	void run(int memory,const char* defaultFile)
	{
		char get[3000]={0};
		char* sen=(char*)malloc(sizeof(char)*memory*1024*1024);
		if(sen==NULL)
			throw NULL;
		memset(sen,0,sizeof(char)*memory*1024*1024);
		if(false==this->bondhost())
		{
			sprintf(this->error,"bond wrong");
			return;
		}
		if(false==this->setlisten())
		{
			sprintf(this->error,"set listen wrong");
			return;
		}
		this->getText=get;
		if(isDebug)
			printf("server is ok\n");
		while(1)
			this->epollHttp(get,3000,sen,defaultFile);
	}
	int httpSend(int num,void* buffer,int sendLen)
	{
		return this->sendSocket(num,buffer,sendLen);
	}
	inline void* recText()
	{
		return this->getText;
	}
	inline const char* lastError()
	{
		return error;
	}
private:
	int func(int num,void* pget,void* sen,const char* defaultFile,HttpServer& server)
	{
		DealHttp http;
		AskType type=GET;
		int len=0,flag=0;
		char ask[200]={0};
		if(strstr((char*)pget,"GET")!=NULL)
		{
			http.getAskRoute(pget,"GET",ask,200);
			if(isDebug)
				printf("url:%s\n",ask);
			type=GET;
		}
		if(strstr((char*)pget,"POST")!=NULL)
		{
			http.getAskRoute(pget,"POST",ask,200);
			if(isDebug)
				printf("url:%s\n",ask);
			type=POST;
		}
		void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&)=NULL;
		for(unsigned int i=0;i<now;i++)
		{
			if(array[i].type==ONEWAY&&(array[i].ask==type||array[i].ask==ALL))
			{
				if(strcmp(ask,array[i].route)==0)
				{
					pfunc=array[i].pfunc;
					break;
				}
			}
			else if(array[i].type==WILD&&(array[i].ask==type||array[i].ask==ALL))
			{
				if(strstr(ask,array[i].route)!=NULL)
				{
					pfunc=array[i].pfunc;
					break;						
				}
			}
		}
		if(pfunc!=NULL)
			pfunc(http,*this,num,sen,len);
		else
		{
			if(isDebug)
				printf("http:%s\n",http.analysisHttpAsk(pget));
			if(http.analysisHttpAsk(pget)!=NULL)
				strcpy(ask,http.analysisHttpAsk(pget));
			flag=http.autoAnalysisGet((char*)pget,(char*)sen,defaultFile,&len);
			if(flag==2&&isDebug)
				printf("404 get %s wrong\n",ask);
		}
		if(false==server.sendSocket(num,sen,len))
		{
			if(isDebug)
				perror("send wrong");
		}
		else
		{
			if(isDebug)
				printf("200 ok send success\n\n");
		}
		return 0;
	}
	void epollHttp(void* pget,int len,void* pneed,const char* defaultFile)
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
		fd_set temp=fdClients;
		int sign=select(0,&temp,NULL,NULL,NULL);
		if(sign>0)
		{
			for(int i=0;i<(int)fdClients.fd_count;i++)
			{
				if(FD_ISSET(fdClients.fd_array[i],&temp))
				{
					if(fdClients.fd_array[i]==sock)
					{
						if(fdClients.fd_count<FD_SETSIZE)
						{
							SOCKADDR_IN newaddr={0};
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(clientIn!=NULL)
								clientIn(*this,newClient,pget,newaddr.sin_port);
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						if(sRec>0)
							func(fdClients.fd_array[i],pget,pneed,defaultFile,*this);
						if(sRec<=0)
						{
							if(this->clientOut!=NULL)
							{
								int port=0;
								strcpy((char*)pget,this->getPeerIp(fdClients.fd_array[i],&port));
								clientOut(*this,fdClients.fd_array[i],pget,port);
							}
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);
							
						}
					}
				}
			}
		}
		else
			return ;
		return ;
	}
};
struct CliMsg{ 
	char hao[20];
	char mi[20];
	int flag;
	char chat[100];
};
struct CliLog{
	int socketCli;
	int time;
	char ip[20];
};
bool findSql(CliMsg cli,char* pname,const char* pfile)
{ 
	char hao[20]={0},mi[20]={0},min[20]={0};
	FILE* fp=fopen(pfile,"r+");
	if(fp==NULL)
		return false;
	while(fscanf(fp,"%s%s%s",hao,mi,min)!=EOF) 
	{
		if(strcmp(hao,cli.hao)==0&&strcmp(mi,cli.mi)==0)
		{
			strcpy(pname,min);
			fclose(fp);
			return true;
		} 	
	}
	fclose(fp);
	return false;
}
bool addSql(CliMsg cli,char* pname,const char* pfile)
{
	FILE* fp=fopen(pfile,"r+");
	if(fp==NULL)
		return false;
	fseek(fp,0,SEEK_END);
	fprintf(fp,"%s %s %s\n",cli.hao,cli.mi,pname);
	fclose(fp);
	return true;
}
bool getFile(char* sen,int* plen,const char* pname)
{
	int i=0;
	FILE* fp=fopen(pname,"r+");
	if(fp==NULL)
		return false;
	fseek(fp,0,SEEK_END);
	*plen=ftell(fp);
	fseek(fp,0,SEEK_SET);	
	for(int j=0;j<*plen;j++)
		sen[i++]=fgetc(fp);
	sen[i]=0;
	fclose(fp);
	return true;
}
char* findBack(char* ps,int len,char* word)
{
	int i=0;
	char* ptemp=ps+len+1;
	char* pend=NULL;
	while(1)
		if((*ptemp>96&&*ptemp<123)||(*ptemp>64&&*ptemp<91)||*ptemp==95)
			break;
		else
			ptemp++;
	pend=ptemp;
	while(1)
		if((*pend>90&&*pend<97&&*pend!=95)||*pend<65||*pend>122)
			break;
		else
			pend++;
	for(char* pi=ptemp;pi<pend;pi++)
		word[i++]=*pi;
	word[i]=0;
	return word;
}
char* findBackString(char* ps,int len,char* word)
{
	int i=0;
	char* ptemp=ps+len+1;
	char* pend=NULL;
	while(1)
		if((*ptemp>47&&*ptemp<58)||(*ptemp>96&&*ptemp<123)||(*ptemp>64&&*ptemp<91)||*ptemp==95)
			break;
		else
			ptemp++;
	pend=ptemp;
	while(1)
		if((*pend>90&&*pend<97&&*pend!=95)||*pend<48||*pend>122||(*pend>57&&*pend<65))
			break;
		else
			pend++;
	for(char* pi=ptemp;pi<pend;pi++)
		word[i++]=*pi;
	word[i]=0;
	return word;
}
char indexName[100]={0};
void chooseModel(unsigned int* port)
{
	printf("please input the import(default 80):");
	scanf("%u",port);
	printf("please input index.html(default welcome.html):");
	scanf("%s",indexName);
}
int funcTwo(int thing,int num,int getLen,void* pget,void* sen,ServerTcpIp& server)
{
	DealHttp http;
	int len=0;
	char ask[200]={0};
	if(sen==NULL)
		return -1;
	memset(sen,0,sizeof(char)*10000000);
	if(thing==0)
	{
		printf("%d out\n",num);

	}
	if(thing==1)
	{
		printf("%d in %s %d\n",num,(char*)pget,getLen);
	}
	if(thing==2)
	{
//		printf("ask:%s",(char*)pget);
		http.cutLineAsk((char*)pget,"GET");
		http.getRecFile(pget,ask,200,(char*)sen,1000000);
		printf("\r\nfile:%s\r\n",(char*)sen);
		if(NULL==http.analysisHttpAsk(pget))
			http.createSendMsg(DealHttp::HTML,(char*)sen,"404.html",&len);
		if(2==http.autoAnalysisGet((char*)pget,(char*)sen,indexName,&len))
		{
			perror("file");	
			printf("some thing wrong %s\n",(char*)pget);
		}
		else
			printf("create auto success\n");
		if(false==server.sendSocket(num,sen,len))
			printf("send wrong\n");
		else
			printf("send success\n\n");
	}
	return 0;
}
void func(DealHttp& http,HttpServer& server,int num,void* sen,int& len)
{
	char buffer[100]={0},name[30]={0};
	
	http.getWildUrl(server.recText(),"/root/",buffer,100);
	http.getRouteValue(buffer,"name",name,30);
	Json json;
	json.jsonInit(300);
	json.addKeyValue("buffer",buffer);
	json.addKeyValInt("wuwu",90);
	json.addKeyValue("name",name);
	printf("json%s\n",json.endJson());
	json.jsonToFile("temp");
	printf("buffer:%s\n",buffer);
	http.createSendMsg(DealHttp::JSON,(char*)sen,"temp",&len);
}
void serverHttp()
{
	unsigned int port=80;
	chooseModel(&port);
//	HttpServer server(port,true);
//		server.routeHandle(HttpServer::GET,HttpServer::WILD,"/login/",func);
//	server.run(10,indexName);
	ServerTcpIp server(port);
	int thing=0,num=0;
	char get[2048]={0};
	char* sen=(char*)malloc(sizeof(char)*10000000);
	if(sen==NULL)
		printf("memory wrong\n");
	if(false==server.bondhost())
		exit(0);
	if(false==server.setlisten())
		exit(0);
	printf("server ip is:%s\nthe server is ok\n",server.getHostIp());
	while(1)
		if(false==server.selectModel(&thing,&num,get,2048,sen,funcTwo))
			break;
	free(sen);
}
int main(int argc, char** argv) 
{
	serverHttp();
	return 0;
}
