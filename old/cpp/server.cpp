/********************************
	author:chenxuan
	date:2021.7.5
	funtion:do not change this file 
*********************************/
#include"../hpp/server.h"
#include"../hpp/http.h"
#include"../hpp/thread.h"
#include<string.h>
#include<iostream>
//linux env
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/epoll.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<signal.h>
#include<unistd.h>
using namespace std;
//linux env
namespace cppweb{
ServerTcpIp::ServerTcpIp(unsigned short port,int epollNum,int wait)
{//port is bound ,epollNum is if open epoll model,wait is listen socket max wait
	sock=socket(AF_INET,SOCK_STREAM,0);//AF=addr family internet
	addr.sin_addr.s_addr=htonl(INADDR_ANY);//inaddr_any
	addr.sin_family=AF_INET;//af_intt IPv4
	addr.sin_port=htons(port);//host to net short
	fd_count=0;// select model
	last_count=fd_count;
	sizeAddr=sizeof(sockaddr);
	backwait=wait;
	numClient=0;
	error=NULL;
	hostip=(char*)malloc(sizeof(char)*200);
	if(hostip==NULL)
		error="hostip worng";
	else
		memset(hostip,0,sizeof(char)*200);
	hostname=(char*)malloc(sizeof(char)*300);
	if(hostname==NULL)
		error="hostname wrong";
	else
		memset(hostname,0,sizeof(char)*300);
	FD_ZERO(&fdClients);//clean fdClients;
	epfd=epoll_create(epollNum);
	if((pevent=(epoll_event*)malloc(512*sizeof(epoll_event)))==NULL)
		error="pevent wrong";
	else
		memset(pevent,0,sizeof(epoll_event)*512);
	memset(&nowEvent,0,sizeof(epoll_event));
	pfdn=(int*)malloc(sizeof(int)*64);
	if(pfdn==NULL)
		error="pfdn wrong";
	else
		memset(pfdn,0,sizeof(int)*64);
	fdNumNow=0;
	fdMax=64;
}
ServerTcpIp::~ServerTcpIp()//clean server
{
	close(sock);
	close(sockC);
	close(epfd);
	if(hostip!=NULL)
		free(hostip);
	if(hostname!=NULL)
		free(hostname);
	if(pevent!=NULL)
		free(pevent);
	if(pfdn!=NULL)
		free(pfdn);
}
bool ServerTcpIp::bondhost()//bond myself first
{
	if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
		return false;
	return true;
}
bool ServerTcpIp::setlisten()//set listem to accept second
{
	if(listen(sock,backwait)==-1)
		return false;
	FD_SET(sock,&fdClients);
	nowEvent.events=EPOLLIN;
	nowEvent.data.fd=sock;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sock,&nowEvent);
	fd_count=sock;
	return true;
}
int ServerTcpIp::acceptClient()//wait until success model one
{
	sockC=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
	return sockC;
}
bool ServerTcpIp::acceptClients(int* pcliNum)//model two
{
	*pcliNum=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
	return this->addFd(*pcliNum);
}
int ServerTcpIp::receiveOne(void* pget,int len)//model one
{
	return recv(sockC,(char*)pget,len,0);
}
int ServerTcpIp::receiveSocket(int clisoc,void* pget,int len)
{
	return recv(clisoc,(char*)pget,len,0);
}
int ServerTcpIp::sendClientOne(const void* psen,int len)//model one
{
	return send(sockC,(char*)psen,len,0);
}
void ServerTcpIp::sendEverySocket(void* psen,int len)
{
	for(int i=0;i<fdNumNow;i++)
		if(pfdn[i]!=0)
			send(pfdn[i],psen,len,0);
}
int ServerTcpIp::sendSocket(int socCli,const void* psen,int len)//send by socket
{
	return send(socCli,(char*)psen,len,0);
}
bool ServerTcpIp::selectModel(void* pget,int len,void* pneed,int (*pfunc)(Thing ,int ,int,void* ,void*,ServerTcpIp& ))
{//num is the num of soc,pget is rec,len is the max len of pget,pneed is others things
	fd_set temp=fdClients;
	Thing pthing=OUT;
	int sign=select(fd_count+1,&temp,NULL,NULL,NULL);
	if(sign>0)
	{
		for(int i=3;i<fd_count+1;i++)
		{
			if(FD_ISSET(i,&temp))
			{
				if(i==sock)
				{
					if(fd_count<1024)
					{
						sockaddr_in newaddr={0,0,{0},{0}};
						int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
						FD_SET(newClient,&fdClients);
						this->addFd(newClient);
						if(newClient>fd_count)
						{
							last_count=fd_count;
							fd_count=newClient;
						}
						strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
						if(pfunc!=NULL)
						{
							if(pfunc(IN,newClient,0,pget,pneed,*this))
								return false;
						}
					}
					else
						continue;
				}
				else
				{
					int sRec=recv(i,(char*)pget,len,0);
					int socRec=i;
					if(sRec>0)
					{
						pthing=SAY;
					}
					if(sRec<=0)
					{
						close(i);
						this->deleteFd(i);
						FD_CLR(i,&fdClients);
						if(i==fd_count)
							fd_count=last_count;
						*(char*) pget=0;
						pthing=OUT;
					}
					if(pfunc!=NULL)
					{
						if(pfunc(pthing,socRec,sRec,pget,pneed,*this))
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
bool ServerTcpIp::updateSocket(int* array,int* pcount)//get epoll array
{
	if(fdNumNow!=0)
		*pcount=fdNumNow;
	else
		return false;
	for(int i=0;i<fdNumNow;i++)
		array[i]=pfdn[i];
	return true;
}
bool ServerTcpIp::findSocket(int cliSoc)//find if socket is connect
{
	for(int i=0;i<fdNumNow;i++)
	{
		if(pfdn[i]==cliSoc)
			return true;
	}
	return false;
}
char* ServerTcpIp::getHostName()//get self name
{
	char name[300]={0};
	gethostname(name,300);
	memcpy(hostname,name,300);
	return hostname;
}
char* ServerTcpIp::getHostIp()//get self ip
{
	char name[300]={0};
	gethostname(name,300);
	hostent* phost=gethostbyname(name);
	in_addr addr;
	char* p=phost->h_addr_list[0];
	memcpy(&addr.s_addr,p,phost->h_length);
	memset(hostip,0,sizeof(char)*200);
	if(strlen(inet_ntoa(addr))>=200)
		return NULL;
	memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
	return hostip;
}
const char* ServerTcpIp::getPeerIp(int cliSoc,int* pcliPort)//get ip and port by socket
{
	sockaddr_in cliAddr={0,0,{0},{0}};
	int len=sizeof(cliAddr);
	if(-1==getpeername(cliSoc,(sockaddr*)&cliAddr,(socklen_t*)&len))
		return NULL;
	*pcliPort=cliAddr.sin_port;
	return inet_ntoa(cliAddr.sin_addr); 
}
bool ServerTcpIp::epollModel(void* pget,int len,void* pneed,int (*pfunc)(Thing,int ,int ,void* ,void*,ServerTcpIp& ))
{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
	Thing thing=SAY;
	int eventNum=epoll_wait(epfd,pevent,512,-1);
	for(int i=0;i<eventNum;i++)
	{
		epoll_event temp=pevent[i];
		if(temp.data.fd==sock)
		{
			sockaddr_in newaddr={0,0,{0},{0}};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
			this->addFd(newClient);
			nowEvent.data.fd=newClient;
			nowEvent.events=EPOLLIN;
			epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
			strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
			if(pfunc!=NULL)
			{
				if(pfunc(IN,newClient,0,pget,pneed,*this))
					return false;
			}
		}
		else
		{
			int getNum=recv(temp.data.fd,(char*)pget,len,0);
			int sockRec=temp.data.fd;
			if(getNum>0)
				thing=SAY;
			else
			{
				*(char*)pget=0;
				thing=OUT;
				this->deleteFd(temp.data.fd);
				epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
				close(temp.data.fd);
			}
			if(pfunc!=NULL)
			{
				if(pfunc(thing,sockRec,getNum,pget,pneed,*this))
					return false;
			}
		}
	}
	return true;
}
bool ServerTcpIp::disconnectSocket(int clisock)//disconnect from socket
{
	close(clisock);
	return this->deleteFd(clisock);
}
bool ServerTcpIp::addFd(int addsoc)
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
bool ServerTcpIp::deleteFd(int clisoc)
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
HttpServer::HttpServer(unsigned port,bool debug,RunModel serverModel,unsigned threadNum)
{
	pool=NULL;
	getText=NULL;
	senText=NULL;
	middleware=NULL;
	defaultFile=NULL;
	arrRoute=NULL;
	clientIn=NULL;
	clientOut=NULL;
	logFunc=NULL;
	logError=NULL;
	pnowRoute=NULL;
	senLen=1;
	recLen=2048;
	selfLen=0;
	boundPort=port;
	isDebug=debug;
	selfCtrl=false;
	isLongCon=true;
	isContinue=true;
	isAutoAnalysis=true;
	textLen=0;
	now=0;
	maxNum=20;
	arrRoute=(RouteFuntion*)malloc(sizeof(RouteFuntion)*20);
	if(arrRoute==NULL)
	{
		this->error=" server route wrong";
		if(logError!=NULL)
			logError(this->error,0);
	}
	else
		memset(arrRoute,0,sizeof(RouteFuntion)*20);
	if(this->model==THREAD)
	{
		if(threadNum==0)
		{
			error="thread num is zero";
			return;
		}
		pool=new ThreadPool(threadNum);
		if(pool==NULL)
		{
			error="pool new wrong";
			return;
		}
	}
	if(this->model==FORK)
		signal(SIGCHLD,sigCliDeal);
}
HttpServer::~HttpServer()
{
	if(arrRoute!=NULL)
		free(arrRoute);
}
bool HttpServer::routeHandle(AskType ask,const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
{//add route handle in all ask type 
	if(strlen(route)>100)
		return false;
	RouteFuntion* nowRoute=addRoute();
	if(nowRoute==NULL)
		return false;
	nowRoute->ask=ask;
	strcpy(nowRoute->route,route);
	nowRoute->pfunc=pfunc;
	if(route[strlen(route)-1]=='*')
	{
		nowRoute->route[strlen(route)-1]=0;
		nowRoute->type=WILD;
	}
	else
		nowRoute->type=ONEWAY;
	if(false==trie.insert(nowRoute->route,nowRoute))
	{
		error="server:route wrong char";
		if(logError!=NULL)
			logError(this->error,0);
		return false;
	}
	return true;
}
bool HttpServer::loadStatic(const char* route,const char* staticFile)
{//load file such as / -> index.html
	if(strlen(route)>100)
		return false;
	RouteFuntion* nowRoute=addRoute();
	if(nowRoute==NULL)
		return false;
	nowRoute->type=STATIC;
	nowRoute->ask=GET;
	strcpy(nowRoute->route,route);
	strcpy(nowRoute->path,staticFile);
	nowRoute->pfunc=loadFile;
	if(false==trie.insert(route,nowRoute))
	{
		error="server:route wrong char";
		if(logError!=NULL)
			logError(this->error,0);
		return false;
	}
	return true;
}
bool HttpServer::loadStaticFS(const char* route,const char* staticPath)
{//load file system such as /tmp to /root
	if(strlen(route)>100)
		return false;
	RouteFuntion* nowRoute=addRoute();
	if(nowRoute==NULL)
		return false;
	nowRoute->type=STAWILD;
	nowRoute->ask=GET;
	strcpy(nowRoute->route,route);
	strcpy(nowRoute->path,staticPath);
	nowRoute->pfunc=loadFile;
	if(false==trie.insert(route,nowRoute))
	{
		error="server:route wrong char";
		if(logError!=NULL)
			logError(this->error,0);
		return false;
	}
	return true;
}
bool HttpServer::deletePath(const char* route)
{// forbidden the file path
	if(strlen(route)>100&&route!=NULL)
		return false;
	RouteFuntion* nowRoute=addRoute();
	if(nowRoute==NULL)
		return false;
	nowRoute->type=STAWILD;
	nowRoute->ask=GET;
	if(route[0]=='.'&&route[1]=='/'&&strlen(route)>2)
		strcpy(nowRoute->route,route+2);
	else if(route[0]!='/')
	{
		strcpy(nowRoute->route,"/");
		strcat(nowRoute->route,route);
	}
	else
		strcpy(nowRoute->route,route);
	nowRoute->pfunc=deleteFile;
	if(false==trie.insert(nowRoute->route,nowRoute))
	{
		error="server:route wrong char";
		if(logError!=NULL)
			logError(this->error,0);
		return false;
	}
	return true;
}
bool HttpServer::get(const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
{//add routeHandle and ask type is get
	if(strlen(route)>100)
		return false;
	RouteFuntion* nowRoute=addRoute();
	if(nowRoute==NULL)
		return false;
	nowRoute->ask=GET;
	nowRoute->pfunc=pfunc;
	strcpy(nowRoute->route,route);
	if(route[strlen(route)-1]=='*')
	{
		nowRoute->route[strlen(route)-1]=0;
		nowRoute->type=WILD;
	}
	else
		nowRoute->type=ONEWAY;
	if(false==trie.insert(nowRoute->route,nowRoute))
	{
		error="server:route wrong char";
		if(logError!=NULL)
			logError(this->error,0);
		return false;
	}
	return true;	
}
bool HttpServer::post(const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
{//the same to last funtion
	if(strlen(route)>100)
		return false;
	RouteFuntion* nowRoute=addRoute();
	if(nowRoute==NULL)
		return false;
	nowRoute->ask=POST;
	strcpy(nowRoute->route,route);
	nowRoute->pfunc=pfunc;
	if(route[strlen(route)-1]=='*')
	{
		nowRoute->route[strlen(route)-1]=0;
		nowRoute->type=WILD;
	}
	else
		nowRoute->type=ONEWAY;
	if(false==trie.insert(nowRoute->route,nowRoute))
	{
		error="server:route wrong char";
		if(logError!=NULL)
			logError(this->error,0);
		return false;
	}
	return true;	
}
bool HttpServer::all(const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
{//receive all ask type
	if(strlen(route)>100)
		return false;
	RouteFuntion* nowRoute=addRoute();
	if(nowRoute==NULL)
		return false;
	nowRoute->ask=ALL;
	strcpy(nowRoute->route,route);
	nowRoute->pfunc=pfunc;
	if(route[strlen(route)-1]=='*')
	{
		nowRoute->route[strlen(route)-1]=0;
		nowRoute->type=WILD;
	}
	else
		nowRoute->type=ONEWAY;
	if(false==trie.insert(nowRoute->route,nowRoute))
	{
		error="server:route wrong char";
		if(logError!=NULL)
			logError(this->error,0);
		return false;
	}
	return true;	
}
bool HttpServer::clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
{//when client in ,it will be call
	if(clientIn!=NULL)
		return false;
	clientIn=pfunc;
	return true;
}
bool HttpServer::clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
{//when client out ,itwill be call
	if(clientOut!=NULL)
		return false;
	clientOut=pfunc;
	return true;
}
bool HttpServer::setMiddleware(void (*pfunc)(HttpServer&,DealHttp&,int))
{//middleware funtion after get text it will be called
	if(middleware!=NULL)
		return false;
	middleware=pfunc;
	return true;
}
bool HttpServer::setLog(void (*pfunc)(const void*,int),void (*errorFunc)(const void*,int))
{//log system 
	if(logFunc!=NULL||logError!=NULL)
		return false;
	logFunc=pfunc;
	logError=errorFunc;
	return true;
}
void HttpServer::run(const char* defaultFile)
{//server begin to run
	char* getT=(char*)malloc(sizeof(char)*recLen);
	char* sen=(char*)malloc(sizeof(char)*senLen*1024*1024);
	if(sen==NULL||getT==NULL)
	{
		this->error="server:malloc get and sen wrong";
		if(logError!=NULL)
			logError(error,0);
		return;
	}
	memset(getT,0,sizeof(char)*recLen);
	memset(sen,0,sizeof(char)*senLen*1024*1024);
	if(false==this->bondhost())
	{
		this->error="server:bound wrong";
		if(logError!=NULL)
			logError(error,0);
		return;
	}
	if(false==this->setlisten())
	{
		this->error="server:set listen wrong";
		if(logError!=NULL)
			logError(error,0);
		return;
	}
	this->getText=getT;
	this->senText=sen;
	this->defaultFile=defaultFile;
	if(isDebug)
		messagePrint();
	switch(model)
	{
	case MULTIPLEXING:
		while(isContinue)
			this->epollHttp();
		break;
	case FORK:
		while(isContinue)
			this->forkHttp();
		break;
	case THREAD:
		while(isContinue)
			this->threadHttp();
		break;
	}
	free(sen);
	free(getT);
}
int HttpServer::getCompleteMessage(int sockCli)
{//some time text is not complete ,it can get left text 
	void*& message=getText;
	unsigned messageLen=this->getRecLen();
	if(message==NULL||message==0)
		return -1;
	unsigned int len=0,old=messageLen;
	char* temp=NULL;
	if((temp=strstr((char*)message,"Content-Length"))==NULL)
		return -1;
	if(sscanf(temp+strlen("Content-Length")+1,"%d",&len)<=0)
		return -1;
	if((temp=strstr((char*)message,"\r\n\r\n"))==NULL)
		return -1;
	temp+=4;
	if(strlen(temp)>=len)
		return messageLen;
	long int leftLen=len-(messageLen-(temp-(char*)message)),getLen=1,all=0;
	while(len+messageLen>recLen)
	{
		recLen*=2;
		message=enlargeMemory(message,recLen);
	}
	unsigned result=messageLen;
	while(leftLen>5&&getLen>0)
	{
		getLen=this->httpRecv(sockCli,(char*)message+old+all,recLen-old-all);
		result+=getLen;
		all+=getLen;
		leftLen-=getLen;
	}
	textLen=result;
	return result;
}
void HttpServer::changeSetting(bool debug,bool isLongCon,bool isAuto,unsigned sendLen)
{//change setting
	this->isDebug=debug;
	this->isLongCon=isLongCon;
	this->isAutoAnalysis=isAuto;
	if(sendLen>0)
		this->senLen=sendLen;
}
void HttpServer::messagePrint()
{
	printf("welcome to web server,the server is runing\n");
	switch(model)
	{
	case MULTIPLEXING:
		printf("model:\t\tIO Multiplexing\n");
		break;
	case THREAD:
		printf("model:\t\tThread Pool\n");
		break;
	case FORK:
		printf("model:\t\tProcess Pool\n");
		break;
	}
	printf("port:\t\t%u\n",boundPort);
	if(isAutoAnalysis)
		printf("auto:\t\tTrue\n");
	else
		printf("auto:\t\tFalse\n");
	if(defaultFile!=NULL)
		printf("/\t\t->\t%s\n",defaultFile);
	for(unsigned i=0;i<now;i++)
	{
		switch(arrRoute[i].type)
		{
		case ONEWAY:
			printf("%s\t\t->\t",arrRoute[i].route);
			break;
		case WILD:
			printf("%s*\t\t->\t",arrRoute[i].route);
			break;
		case STATIC:
		case STAWILD:
			if(arrRoute[i].pfunc==loadFile)
				printf("%s\t\t->%s\n",arrRoute[i].route,arrRoute[i].path);
			else if(arrRoute[i].pfunc==deleteFile)
				printf("%s\t\t->\tdelete\n",arrRoute[i].route);
			else
				printf("undefine funtion please check the server\n");
			continue;
		}
		switch(arrRoute[i].ask)
		{
		case GET:
			printf("GET\n");
			break;
		case POST:
			printf("POST\n");
			break;
		case ALL:
			printf("All\n");
			break;
		case PUT:
			printf("PUT\n");
			break;
		case DELETE:
			printf("DELETE\n");
			break;
		case OPTIONS:
			printf("OPTIONS\n");
			break;
		case CONNECT:
			printf("CONNECT\n");
			break;
		}
	}
	if(middleware!=NULL)
		printf("middleware funtion set\n");
	if(logFunc!=NULL)
		printf("log function set\n");
	if(logError!=NULL)
		printf("error funtion set\n");
	if(clientIn!=NULL)
		printf("client in function set\n");
	if(clientOut!=NULL)
		printf("client out function set\n");
	printf("\n");
}
int func(int num)
{
	static DealHttp http;
	AskType type=GET;
	int len=0,flag=2;
	char ask[200]={0};
	if(middleware!=NULL)
	{
		middleware(*this,http,num);
		return 0;
	}
	if(isLongCon==false)
		http.changeSetting("Close","LCserver/1.1");
	sscanf((char*)this->getText,"%100s",ask);
	if(strstr(ask,"GET")!=NULL)
	{
		http.getAskRoute(this->getText,"GET",ask,200);
		if(isDebug)
			printf("Get url:%s\n",ask);
		type=GET;
	}
	else if(strstr(ask,"POST")!=NULL)
	{
		http.getAskRoute(this->getText,"POST",ask,200);
		if(isDebug)
			printf("POST url:%s\n",ask);
		type=POST;
	}
	else if(strstr(ask,"PUT")!=NULL)
	{
		http.getAskRoute(this->getText,"PUT",ask,200);
		if(isDebug)
			printf("PUT url:%s\n",ask);
		type=PUT;
	}
	else if(strstr(ask,"DELETE")!=NULL)
	{
		http.getAskRoute(this->getText,"DELETE",ask,200);
		if(isDebug)
			printf("DELETE url:%s\n",ask);
		type=DELETE;
	}
	else if(strstr(ask,"OPTIONS")!=NULL)
	{
		http.getAskRoute(this->getText,"OPTIONS",ask,200);
		if(isDebug)
			printf("OPTIONS url:%s\n",ask);
		type=OPTIONS;
	}
	else if(strstr(ask,"CONNECT")!=NULL)
	{
		http.getAskRoute(this->getText,"CONNECT",ask,200);
		if(isDebug)
			printf("CONNECT url:%s\n",ask);
		type=CONNECT;
	}
	else 
	{
		memset(ask,0,sizeof(char)*200);
		if(isDebug)
			printf("way not support\n");
		type=GET;
	}
	void (*pfunc)(HttpServer&,DealHttp&,int)=NULL;
	RouteFuntion* tempRoute=trie.search(ask,[=](const RouteFuntion* now,bool isLast)->bool{
				if(now->ask==ALL||now->ask==type)
				{
					if(isLast&&(now->type==STATIC||now->type==ONEWAY))
						return true;
					else if(now->type==WILD||now->type==STAWILD)
						return true;
					else
						return false;
				}
				return false;
				});
	if(tempRoute!=NULL)
	{
		pnowRoute=tempRoute;
		pfunc=tempRoute->pfunc;
	}
	if(pfunc!=NULL)
	{
		if(pfunc!=loadFile&&pfunc!=deleteFile)
		{
			http.gram.body="";
			http.gram.cookie.clear();
			http.gram.head.clear();
			http.gram.fileLen=0;
			http.gram.typeFile=DealHttp::TXT;
			pfunc(*this,http,num);
			if(selfCtrl)
			{
				selfCtrl=false;
				len=selfLen;
				selfLen=0;
			}
			else
			{
				if(http.gram.fileLen==0)
					http.gram.fileLen=http.gram.body.size();
				len=http.createDatagram(http.gram,this->senText,this->senLen*1024*1024);
				while(len==-1)
				{
					this->senText=enlargeMemory(this->senText,this->senLen);
					len=http.createDatagram(http.gram,this->senText,this->senLen*1024*1024);
				}
			}
		}
		else
		{
			pfunc(*this,http,senLen);
			len=staticLen(-1);
		}
	}
	else if(isAutoAnalysis)
	{
		if(isDebug)
			printf("http:%s\n",http.analysisHttpAsk(this->getText));
		if(http.analysisHttpAsk(this->getText)!=NULL)
		{
			strcpy(ask,http.analysisHttpAsk(this->getText));
			flag=http.autoAnalysisGet((char*)this->getText,(char*)this->senText,senLen*1024*1024,defaultFile,&len);
		}
		if(flag==2)
		{
			if(isDebug)
				printf("404 get %s wrong\n",ask);
			if(logError!=NULL)
				logError(ask,num);
		}
	}
	else
		http.createSendMsg(DealHttp::NOFOUND,(char*)this->senText,senLen*1024*1024,NULL,&len);
	if(len==0)
		http.createSendMsg(DealHttp::NOFOUND,(char*)this->senText,senLen*1024*1024,NULL,&len);
	if(0>=this->sendSocket(num,this->senText,len))
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
void HttpServer::epollHttp()
{//pthing is 0 out,1 in,2 say pnum is the num of soc,this->getText is rec,len is the max len of this->getText,pneed is others things
	memset(this->getText,0,sizeof(char)*this->recLen);
	int eventNum=epoll_wait(epfd,pevent,512,-1);
	for(int i=0;i<eventNum;i++)
	{
		epoll_event temp=pevent[i];
		if(temp.data.fd==sock)
		{
			sockaddr_in newaddr={0,0,{0},{0}};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
			this->addFd(newClient);
			nowEvent.data.fd=newClient;
			nowEvent.events=EPOLLIN;
			epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
			if(this->clientIn!=NULL)
			{
				strcpy((char*)this->getText,inet_ntoa(newaddr.sin_addr));
				clientIn(*this,newClient,this->getText,newaddr.sin_port);
			}
		}
		else
		{
			int getNum=recv(temp.data.fd,(char*)this->getText,this->recLen,MSG_DONTWAIT);
			int all=getNum;
			while((int)this->recLen==all)
			{
				this->getText=enlargeMemory(this->getText,this->recLen);
				getNum=recv(temp.data.fd,(char*)this->getText+all,this->recLen-all,MSG_DONTWAIT);
				if(getNum<=0)
					break;
				all+=getNum;
			}
			if(all>0)
			{
				this->textLen=all;
				func(temp.data.fd);
				if(logFunc!=NULL)
					logFunc(this->recText(),temp.data.fd);
				if(isLongCon==false)
				{
			  		this->deleteFd(temp.data.fd);
					epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
					close(temp.data.fd);						
				}
			}
			else
			{
				if(this->clientOut!=NULL)
				{
					int port=0;
					strcpy((char*)this->getText,this->getPeerIp(temp.data.fd,&port));
					clientOut(*this,temp.data.fd,this->getText,port);
				}
				this->deleteFd(temp.data.fd);
				epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
				close(temp.data.fd);
			}
		}
	}
	return ;
}
void HttpServer::forkHttp()
{
	memset(this->getText,0,sizeof(char)*this->recLen);
	int eventNum=epoll_wait(epfd,pevent,512,-1);
	for(int i=0;i<eventNum;i++)
	{
		epoll_event temp=pevent[i];
		if(temp.data.fd==sock)
		{
			sockaddr_in newaddr={0,0,{0},{0}};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
			this->addFd(newClient);
			nowEvent.data.fd=newClient;
			nowEvent.events=EPOLLIN|EPOLLET;
			epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
			if(this->clientIn!=NULL)
			{
				strcpy((char*)this->getText,inet_ntoa(newaddr.sin_addr));
				clientIn(*this,newClient,this->getText,newaddr.sin_port);
			}
		}
		else
		{
			int getNum=recv(temp.data.fd,(char*)this->getText,sizeof(char)*this->recLen,0);
			int all=getNum;
			while((int)this->recLen==all)
			{
				this->getText=enlargeMemory(this->getText,this->recLen);
				getNum=recv(temp.data.fd,(char*)this->getText+all,this->recLen-all,MSG_DONTWAIT);
				if(getNum<=0)
					break;
				all+=getNum;
			}
			if(all>0)
			{
				this->textLen=all;
				if(fork()==0)
				{
					close(sock);
					func(temp.data.fd);
					close(temp.data.fd);
					free(this->getText);
					free(this->senText);
					exit(0);
				}
				else
				{
					if(isLongCon==false)
					{
				  		this->deleteFd(temp.data.fd);
						epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
						close(temp.data.fd);
					}
				}
				if(logFunc!=NULL)
					logFunc(this->recText(),temp.data.fd);
			}
			else
			{
				if(this->clientOut!=NULL)
				{
					int port=0;
					strcpy((char*)this->getText,this->getPeerIp(temp.data.fd,&port));
					clientOut(*this,temp.data.fd,this->getText,port);
				}
				this->deleteFd(temp.data.fd);
				epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
				close(temp.data.fd);
			}
		}
	}
	return ;
}
RouteFuntion* HttpServer::addRoute()
{
	RouteFuntion* temp=NULL; 
	if(maxNum-now<=2)
	{
		temp=(RouteFuntion*)realloc(arrRoute,sizeof(RouteFuntion)*(maxNum+10));
		if(temp==NULL)
			return NULL;
		arrRoute=temp;
		maxNum+=10;
	}
	temp=arrRoute+now;
	now++;
	return temp;
}
void HttpServer::threadHttp()
{
	while(this->isContinue)
	{
		sockaddr_in newaddr={0,0,{0},{0}};
		int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
		if(newClient==-1)
			continue;
		ThreadArg* temp=new ThreadArg;
		temp->pserver=this;
		temp->soc=newClient;
		ThreadPool::Task task={threadWorker,temp};
		pool->addTask(task);
	}
}
void* HttpServer::threadWorker(void* self)
{
	ThreadArg* argv=(ThreadArg*)self;
	HttpServer& server=*argv->pserver;
	int cli=argv->soc;
	char* rec=(char*)malloc(sizeof(char)*server.recLen);
	unsigned int size=sizeof(char)*server.recLen;
	if(rec==NULL)
	{
		close(cli);
		free(self);
		return NULL;
	}
	memset(rec,0,sizeof(char)*server.recLen);
	if(server.clientIn!=NULL)
	{
		int port=0;
		server.getPeerIp(cli,&port);
		server.clientIn(server,cli,server.senText,port);
	}
	int recLen=server.receiveSocket(cli,rec,size);
	int all=recLen;
	while((int)size==all)
	{
		rec=(char*)server.enlargeMemory(rec,size);
		recLen=server.receiveSocket(cli,rec,size);
		if(recLen<=0)
			break;
		all+=recLen;
	}
	while(all>=10)
	{
		server.pool->mutexLock();
		server.textLen=all;
		server.getText=rec;
		server.func(cli);
		if(server.logFunc!=NULL)
			server.logFunc(rec,cli);
		server.getText=NULL;
		server.pool->mutexUnlock();
		if(server.isLongCon==false)
			break;
		memset(rec,0,sizeof(char)*server.recLen);
		all=server.receiveSocket(cli,rec,size);
	}
	if(server.clientOut!=NULL)
	{
		int port=0;
		strcpy((char*)server.senText,server.getPeerIp(cli,&port));
		server.clientOut(server,cli,server.senText,port);
	}
	free(rec);
	free(self);
	close(cli);
	return NULL;
}
void HttpServer::loadFile(HttpServer& server,DealHttp& http,int senLen)
{
	int len=0;
	char ask[200]={0},buf[500]={0},temp[200]={0};
	http.getAskRoute(server.recText(),"GET",ask,200);
	HttpServer::RouteFuntion& route=*server.getNowRoute();
	http.getWildUrl(ask,route.route,temp,200);
	sprintf(buf,"GET %s%s HTTP/1.1",route.path,temp);
	if(2==http.autoAnalysisGet(buf,(char*)server.getSenBuff(),senLen*1024*1024,NULL,&len))
	{
		if(server.logError!=NULL)
			server.logError(server.error,0);
		if(server.isDebug)
			printf("404 get %s wrong\n",buf);
	}
	staticLen(len);
}
void HttpServer::deleteFile(HttpServer& server,DealHttp& http,int senLen)
{
	int len=0;
	http.customizeAddTop(server.getSenBuff(),senLen*1024*1024,DealHttp::STATUSFORBIDDEN,strlen("403 forbidden"),"text/plain");
	http.customizeAddBody(server.getSenBuff(),senLen*1024*1024,"403 forbidden",strlen("403 forbidden"));
	len=strlen((char*)server.getSenBuff());
	staticLen(len);
}
unsigned HttpServer::staticLen(int senLen)
{
	static unsigned len=0;
	if(senLen<0)
		return len;
	else
		len=senLen;
	return 0;
}
void* HttpServer::enlargeMemory(void* old,unsigned& oldSize)
{
	oldSize*=2;
	void* temp=realloc(old,oldSize);
	if(temp==NULL)
	{
		error="server:malloc wrong";
		if(logError!=NULL)
			logError(error,0);
		oldSize/=2;
		return old;
	}
	else
		return temp;
}
void HttpServer::sigCliDeal(int )
{
	while(waitpid(-1, NULL, WNOHANG)>0);
}

Email::Email(const char* domain,bool debug)
{
	isDebug=debug;
	memset(error,0,sizeof(char)*30);
	memset(&their_addr, 0, sizeof(their_addr));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(25);
	hostent* hptr = gethostbyname(domain);
	memcpy(&their_addr.sin_addr.s_addr, hptr->h_addr_list[0], hptr->h_length);
}
bool Email::emailSend(const char* sendEmail,const char* passwd,const char* recEmail,const char* body)
{
  	int sockfd = 0;
  	char recBuffer[1000]={0},senBuffer[1000]={0},login[128],pass[128];
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		sprintf(error,"open socket wrong");
		return false;
	}
	if (connect(sockfd,(struct sockaddr *)&their_addr, sizeof(struct sockaddr)) < 0)
	{
		sprintf(error,"connect wrong");
		return false;
	}
	memset(recBuffer,0,sizeof(char)*1000);
	while (recv(sockfd, recBuffer, 1000, 0) <= 0)
	{
		if(isDebug)
			printf("reconnecting\n");
		sleep(2);
		sockfd = socket(PF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			sprintf(error,"open socket wrong");
			return false;
		}
		if (connect(sockfd,(struct sockaddr *)&their_addr, sizeof(struct sockaddr)) < 0)
		{
			sprintf(error,"connect wrong");
			return false;
		}
		memset(recBuffer, 0, 1000);
	}
	if(isDebug)
		printf("get:%s",recBuffer);
	
	memset(senBuffer, 0, 1000);
	sprintf(senBuffer, "EHLO HYL-PC\r\n");
	send(sockfd, senBuffer, strlen(senBuffer), 0);
	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("EHLO REceive:%s\n",recBuffer);
	
	memset(senBuffer, 0, 1000);
	sprintf(senBuffer, "AUTH LOGIN\r\n");
	send(sockfd, senBuffer, strlen(senBuffer), 0);
	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("Auth Login Receive:%s\n",recBuffer);
	memset(senBuffer, 0, 1000);
	sprintf(senBuffer, "%s",sendEmail);
	memset(login, 0, 128);
	EncodeBase64(login, senBuffer, strlen(senBuffer));
	sprintf(senBuffer, "%s\r\n", login);
	send(sockfd, senBuffer, strlen(senBuffer), 0);
	if(isDebug)
		printf("Base64 UserName:%s\n",senBuffer);
	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("User Login Receive:%s\n",recBuffer);

	sprintf(senBuffer, "%s",passwd);//password
	memset(pass, 0, 128);
	EncodeBase64(pass, senBuffer, strlen(senBuffer));
	sprintf(senBuffer, "%s\r\n", pass);
	send(sockfd, senBuffer, strlen(senBuffer), 0);
	if(isDebug)
		printf("Base64 Password:%s\n",senBuffer);

	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("Send Password Receive:%s\n",recBuffer);

	// self email
	memset(recBuffer, 0, 1000);
	sprintf(senBuffer, "MAIL FROM: <%s>\r\n",sendEmail);  
	send(sockfd, senBuffer, strlen(senBuffer), 0);
	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("set Mail From Receive:%s\n",recBuffer);

	// recv email
	sprintf(recBuffer, "RCPT TO:<%s>\r\n", recEmail);
	send(sockfd, recBuffer, strlen(recBuffer), 0);
	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("Tell Sendto Receive:%s\n",recBuffer);
	int bug=0;
	sscanf(recBuffer,"%d",&bug);
	if(bug==550)
	{
		sprintf(error,"recemail wrong");
		return false;
	}
	// send body
	sprintf(senBuffer, "DATA\r\n");
	send(sockfd, senBuffer, strlen(senBuffer), 0);
	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("Send Mail Prepare Receive:%s\n",recBuffer);

	// send data
	sprintf(senBuffer, "%s\r\n.\r\n", body);
	send(sockfd, senBuffer, strlen(senBuffer), 0);
	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("Send Mail Receive:%s\n",recBuffer);

	// QUIT
	sprintf(senBuffer,"QUIT\r\n");
	send(sockfd, senBuffer, strlen(senBuffer), 0);
	memset(recBuffer, 0, 1000);
	recv(sockfd, recBuffer, 1000, 0);
	if(isDebug)
		printf("Quit Receive:%s\n",recBuffer);
	return true;
}
char Email::ConvertToBase64(char uc)
{
	if (uc < 26)
		return 'A' + uc;
	if (uc < 52)
		return 'a' + (uc - 26);
	if (uc < 62)
		return '0' + (uc - 52);
	if (uc == 62)
		return '+';
	return '/';
}
void Email::EncodeBase64(char *dbuf, char *buf128, int len)
{
	struct Base64Date6 *ddd = NULL;
	int i = 0;
	char buf[256] = { 0 };
	char* tmp = NULL;
	char cc = '\0';
	memset(buf, 0, 256);
	strcpy(buf, buf128);
	for (i = 1; i <= len / 3; i++)
	{
		tmp = buf + (i - 1) * 3;
		cc = tmp[2];
		tmp[2] = tmp[0];
		tmp[0] = cc;
		ddd = (struct Base64Date6 *)tmp;
		dbuf[(i - 1) * 4 + 0] = ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i - 1) * 4 + 1] = ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i - 1) * 4 + 2] = ConvertToBase64((unsigned int)ddd->d3);
		dbuf[(i - 1) * 4 + 3] = ConvertToBase64((unsigned int)ddd->d4);
	}
	if (len % 3 == 1)
	{
		tmp = buf + (i - 1) * 3;
		cc = tmp[2];
		tmp[2] = tmp[0];
		tmp[0] = cc;
		ddd = (struct Base64Date6 *)tmp;
		dbuf[(i - 1) * 4 + 0] = ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i - 1) * 4 + 1] = ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i - 1) * 4 + 2] = '=';
		dbuf[(i - 1) * 4 + 3] = '=';
	}
	if (len % 3 == 2)
	{
		tmp = buf + (i - 1) * 3;
		cc = tmp[2];
		tmp[2] = tmp[0];
		tmp[0] = cc;
		ddd = (struct Base64Date6 *)tmp;
		dbuf[(i - 1) * 4 + 0] = ConvertToBase64((unsigned int)ddd->d1);
		dbuf[(i - 1) * 4 + 1] = ConvertToBase64((unsigned int)ddd->d2);
		dbuf[(i - 1) * 4 + 2] = ConvertToBase64((unsigned int)ddd->d3);
		dbuf[(i - 1) * 4 + 3] = '=';
	}
	return;
}
void Email::CreateSend(const char* youName,const char* toName,const char* from,const char* to,const char* subject,const char* body,char* buf)
{
	sprintf(buf,"From: \"%s\"<%s>\r\n"
			"To: \"%s\"<%s>\r\n"
			"Subject:%s\r\n\r\n"
			"%s\n",youName,from,toName,to,subject,body);
}
const char* Email::getDomainBySelfEmail(const char* email,char* buffer,int bufferLen)
{
	char* temp=strrchr((char*)email,'@');
	if(temp==NULL)
		return NULL;
	if(bufferLen<=20)
		return NULL;
	if(strlen(temp)>15)
		return NULL;
	sprintf(buffer,"smtp.%s",temp+1);
	return buffer;
}
/********************************
	author:chenxuan
	date:2021/8/10
	funtion:class for client linux
*********************************/
ClientTcpIp::ClientTcpIp(const char* hostIp,unsigned short port)
{
	memset(ip,0,100);
	memset(selfIp,0,100);
	hostip=(char*)malloc(sizeof(char)*50);
	if(hostip==NULL)
	{
		error="malloc wrong";
		return;
	}
	memset(hostip,0,sizeof(char)*50);
	hostname=(char*)malloc(sizeof(char)*50);
	if(hostname==NULL)
	{
		error="malloc wrong";
		return;
	}	
	memset(hostname,0,sizeof(char)*50);
	if(hostIp!=NULL)
		strcpy(ip,hostIp);
	sock=socket(AF_INET,SOCK_STREAM,0);
	if(hostIp!=NULL)
		addrC.sin_addr.s_addr=inet_addr(hostIp);
	addrC.sin_family=AF_INET;//af_intt IPv4
	addrC.sin_port=htons(port);
	error=NULL;
//	ssl=NULL;
//	ctx=NULL;
}
ClientTcpIp::~ClientTcpIp()
{
//	if(ssl!=NULL)
//	{
//		SSL_shutdown(ssl);
//		SSL_free(ssl);	
//	}
//	if(ctx!=NULL)
//		SSL_CTX_free(ctx);
	free(hostip);
	free(hostname);
	close(sock);
}
void ClientTcpIp::addHostIp(const char* ip,unsigned short port)
{
	if(ip==NULL)
		return;
	strcpy(this->ip,ip);
	addrC.sin_addr.s_addr=inet_addr(ip);
	if(port!=0)
		addrC.sin_port=htons(port);
}
bool ClientTcpIp::tryConnect()
{
	if(connect(sock,(sockaddr*)&addrC,sizeof(sockaddr))==-1)
		return false;
	return true;
}
bool ClientTcpIp::disconnectHost()
{
	close(sock);
	sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
		return false;
	return true;
}
char* ClientTcpIp::getSelfIp()
{
	char name[300]={0};
	gethostname(name,300);
	hostent* phost=gethostbyname(name);
	in_addr addr;
	char* p=phost->h_addr_list[0];
	memcpy(&addr.s_addr,p,phost->h_length);
	memset(selfIp,0,sizeof(char)*100);
	memcpy(selfIp,inet_ntoa(addr),strlen(inet_ntoa(addr)));
	return selfIp;
}
char* ClientTcpIp::getSelfName(char* hostname,unsigned int bufferLen)
{
	char name[300]={0};
	gethostname(name,300);
	if(strlen(name)>=bufferLen)
		return NULL;
	memcpy(hostname,name,strlen(name));
	return hostname;
}
bool ClientTcpIp::getDnsIp(const char* name,char* ip,unsigned int ipMaxLen)
{
	hostent* phost=gethostbyname(name);
	if(phost==NULL)
		return false;
	in_addr addr;
	char* p=phost->h_addr_list[0];
	memcpy(&addr.s_addr,p,phost->h_length);
	if(strlen(inet_ntoa(addr))>=ipMaxLen)
		return false;
	strcpy(ip,inet_ntoa(addr));
	return true;
}
}
