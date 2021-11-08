#ifndef _NETSOCKET_H
#define _NETSOCKET_H
#endif
#include<winsock2.h>
#include<queue>
#include<pthread.h>
using namespace std;
class ServerTcpIp{
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
protected:
    int* pfdn;//pointer if file descriptor
    int fdNumNow;//num of fd now
    int fdMax;//fd max num
    bool addFd(int addsoc);
    bool deleteFd(int clisoc);
public:
	ServerTcpIp(unsigned short port=5200,int wait=5);
	~ServerTcpIp();//clean server
	bool bondhost();//bond myself
	bool setlisten();//set listem to accept
	unsigned int acceptClient();//wait until success
	bool acceptClients(unsigned int* psock);//model two
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
	void sendEverySocket(const void* psen,int len);//model two
	bool selectModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIp& ));
	bool selectSendEveryone(void* psend,int len);
	bool updateSocketSelect(SOCKET* psocket,int* pcount);
	bool sendSocketSelect(SOCKET toClient,const void* psend,int len);
	SOCKET findSocketSelect(int i);
    bool disconnectSocket(SOCKET clisock);
	bool updateSocket(SOCKET* array,int* pcount);
	bool findSocket(int cliSoc);
	char* getHostIp();
	char* getHostName();
	char* getPeerIp(SOCKET cliSoc,int* pcliPort);
};
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
public:
	ClientTcpIp(const char* hostIp,int port=5200);
	~ClientTcpIp();
	void addHostIp(const char* ip);
	bool tryConnect();
	inline int receiveHost(void* prec,int len)
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendHost(const void* psend,int len)
	{
		return send(sock,(char*)psend,len,0);
	}
	bool disconnectHost();
	char* getSelfIp();
	char* getSelfName();
	bool getDnsIp(const char* name,char* ip);
};
class DealHttp{
private:
	char ask[256];
	char* pfind;
public:
	enum FileKind{
		UNKNOWN=0,HTML=1,EXE=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP7=7
	};
public:
	DealHttp();
	bool cutLineAsk(char* pask,const char* pcutIn);
	const char* analysisHttpAsk(void* pask,const char* pneed="GET",int needLen=3);
	inline char* findFirst(void* pask,const char* ptofind)
	{
		return strstr((char*)pask,ptofind);
	}
	char* findBackString(char* local,int len,char* word,int maxWordLen);
	void createTop(FileKind kind,char* ptop,int* topLen,int fileLen);
	bool createSendMsg(FileKind kind,char* pask,const char* pfile,int* plong);
	char* findFileMsg(const char* pname,int* plen,char* buffer);
	int getFileLen(const char* pname);
	int autoAnalysisGet(const char* pask,char* psend,const char* pfirstFile,int* plen);
	const char* getKeyValue(const void* message,const char* key,char* value,int maxValueLen);
	const char* getKeyLine(const void* message,const char* key,char* line,int maxLineLen);
	static void dealUrl(const char* url,char* urlTop,char* urlEnd);
};
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
	static void* worker(void* arg);//the worker for user 
	static void* manager(void* arg);//manager for user
public:
	ThreadPool(unsigned int threadNum=10);//create threads
	~ThreadPool();//destory pool
	void threadExit();// a no use funtion
	void addTask(Task task);//by this you can add task
	void endPool();//user delete the pool
	void getBusyAndTask(unsigned int* pthread,unsigned int* ptask);//get busy live and task num
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
};
class FileGet{
private:
	char* pbuffer;
public:
	FileGet();
	~FileGet();
	int getFileLen(const char* fileName);
	bool getFileMsg(const char* fileName,char* buffer);
	bool fileStrstr(const char* fileName,const char* strFind);
};
