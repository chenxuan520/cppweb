/********************************
	author:chenxuan
	date:2021.7.5
	funtion:do not change this file 
*********************************/
#include"../hpp/server.h"
#include"../hpp/http.h"
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<iostream>
using namespace std;
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
	hostip=(char*)malloc(sizeof(char)*200);
	memset(hostip,0,sizeof(char)*200);
	hostname=(char*)malloc(sizeof(char)*300);
	memset(hostname,0,sizeof(char)*300);
	FD_ZERO(&fdClients);//clean fdClients;
	epfd=epoll_create(epollNum);
	if((pevent=(epoll_event*)malloc(512*sizeof(epoll_event)))==NULL)
		throw NULL;
	memset(pevent,0,sizeof(epoll_event)*512);
	memset(&nowEvent,0,sizeof(epoll_event));
    pfdn=(int*)malloc(sizeof(int)*64);
    if(pfdn==NULL)
        throw NULL;
    memset(pfdn,0,sizeof(int)*64);
    fdNumNow=0;
    fdMax=64;
}
ServerTcpIp::~ServerTcpIp()//clean server
{
	close(sock);
	close(sockC);
	close(epfd);
	free(hostip);
	free(hostname);
	free(pevent);
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
bool ServerTcpIp::selectModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int,void* ,void*,ServerTcpIp& ))
{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
	fd_set temp=fdClients;
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
						sockaddr_in newaddr={0};
						int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
						FD_SET(newClient,&fdClients);
						this->addFd(newClient);
						if(newClient>fd_count)
						{
							last_count=fd_count;
							fd_count=newClient;
						}
						*pnum=newClient;
						*pthing=1;
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
					int sRec=recv(i,(char*)pget,len,0);
					*pnum=i;
					if(sRec>0)
					{
						*pthing=2;
					}
					if(sRec<=0)
					{
						close(i);
						this->deleteFd(i);
						FD_CLR(i,&fdClients);
						if(i==fd_count)
							fd_count=last_count;
						*(char*) pget=0;
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
	memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
	return hostip;
}
char* ServerTcpIp::getPeerIp(int cliSoc,int* pcliPort)//get ip and port by socket
{
	sockaddr_in cliAddr={0};
	int len=sizeof(cliAddr);
	if(-1==getpeername(cliSoc,(sockaddr*)&cliAddr,(socklen_t*)&len))
		return NULL;
	*pcliPort=cliAddr.sin_port;
	return inet_ntoa(cliAddr.sin_addr); 
}
bool ServerTcpIp::epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIp& ))
{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
	int eventNum=epoll_wait(epfd,pevent,512,-1);
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
		{
			int getNum=recv(temp.data.fd,(char*)pget,len,0);
			*pnum=temp.data.fd;
			if(getNum>0)
				*pthing=2;
			else
			{
				*(char*)pget=0;
				*pthing=0;
                this->deleteFd(temp.data.fd);
				epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
				close(temp.data.fd);
			}
			if(pfunc!=NULL)
			{
				if(pfunc(*pthing,*pnum,getNum,pget,pneed,*this))
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
HttpServer::HttpServer(unsigned port,bool debug):ServerTcpIp(port)
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
}
HttpServer::~HttpServer()
{
	if(array!=NULL)
		free(array);
	if(error!=NULL)
		free(error);
}
bool HttpServer::routeHandle(AskType ask,RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
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
const char* HttpServer::getWildUrl(const char* route,char* buffer,int maxLen)
{
	char* temp=strstr((char*)this->getText,route);
	if(temp==NULL)
		return NULL;
	temp+=strlen(route);
	sscanf(temp,"%s",buffer);
}
void HttpServer::run(int memory,const char* defaultFile)
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
int HttpServer::httpSend(int num,void* buffer,int sendLen)
{
	return this->sendSocket(num,buffer,sendLen);
}
int HttpServer::func(int num,void* pget,void* sen,const char* defaultFile,HttpServer& server)
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
		strcpy(ask,http.analysisHttpAsk(pget));
		flag=http.autoAnalysisGet((char*)pget,(char*)sen,defaultFile,&len);
	}
	if(false==server.sendSocket(num,sen,len))
	{
		if(isDebug)
			perror("send wrong");
	}
	else
	{
		if(isDebug)
			printf("send success\n");
	}
	return 0;
}
void HttpServer::epollHttp(void* pget,int len,void* pneed,const char* defaultFile)
{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
	int eventNum=epoll_wait(epfd,pevent,512,-1);
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
		}
		else
		{
			int getNum=recv(temp.data.fd,(char*)pget,len,0);
			if(getNum>0)
				func(temp.data.fd,pget,pneed,defaultFile,*this);
			else
			{
                this->deleteFd(temp.data.fd);
				epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
				close(temp.data.fd);
			}
		}
	}
	return ;
}
