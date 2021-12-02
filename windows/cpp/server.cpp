/********************************
	author:chenxuan
	date:2021.7.5
	funtion:do not change this file 
*********************************/
#include"../hpp/server.h"
#include"../hpp/http.h"
#include<string.h>
#include<iostream>
#include<winsock2.h>
using namespace std;

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
ServerTcpIp::ServerTcpIp(unsigned short port,int epollNum,int wait)
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
ServerTcpIp::~ServerTcpIp()//clean server
{
	closesocket(sock);
	closesocket(sockC);
	free(pfdn);
	free(hostip);
	free(hostname);
	WSACleanup();
}
bool ServerTcpIp::bondhost()//bond myself
{
	if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
		return false;
	return true;
}
bool ServerTcpIp::setlisten()//set listem to accept
{
	if(listen(sock,backwait)==SOCKET_ERROR)
		return false;
	FD_SET(sock,&fdClients);
	return true;
}
unsigned int ServerTcpIp::acceptClient()//wait until success
{
	sockC=accept(sock,(sockaddr*)&client,&sizeAddr);
	return sockC;
}
bool ServerTcpIp::acceptClients(unsigned int* psock)//model two
{
	*psock=accept(sock,(sockaddr*)&client,&sizeAddr);
	return this->addFd(*psock);
}
void ServerTcpIp::sendEverySocket(const void* psen,int len)//model two
{
	for(int i=0;i<fdNumNow;i++)
		if(pfdn[i]!=0)
		    send(pfdn[i],(char*)psen,len,0);
}
bool ServerTcpIp::epollModel(void* pget,int len,void* pneed,int (*pfunc)(int,int ,int ,void* ,void*,ServerTcpIp& ))
{//0 out,1 in,2 say
	fd_set temp=fdClients;
	int thing=2;
	int num=0;
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
						thing=1;
						num=newClient;
						strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
						if(pfunc!=NULL)
						{
							if(pfunc(thing,num,0,pget,pneed,*this))
								return false;
						}
					}
					else
						continue;
				}
				else
				{
					int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
					num=fdClients.fd_array[i];
					if(sRec>0)
						thing=2;
					if(sRec<=0)
					{
						closesocket(fdClients.fd_array[i]);
						FD_CLR(fdClients.fd_array[i],&fdClients);
						this->deleteFd(fdClients.fd_array[i]);
						thing=0;
					}
					if(pfunc!=NULL)
					{
						if(pfunc(thing,num,sRec,pget,pneed,*this))
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
bool ServerTcpIp::selectModel(void* pget,int len,void* pneed,int (*pfunc)(int,int ,int ,void* ,void*,ServerTcpIp& ))
{//0 out,1 in,2 say
	fd_set temp=fdClients;
	int thing=2;
	int num=0;
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
						thing=1;
						num=newClient;
						strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
						if(pfunc!=NULL)
						{
							if(pfunc(thing,num,0,pget,pneed,*this))
								return false;
						}
					}
					else
						continue;
				}
				else
				{
					int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
					num=fdClients.fd_array[i];
					if(sRec>0)
						thing=2;
					if(sRec<=0)
					{
						closesocket(fdClients.fd_array[i]);
						FD_CLR(fdClients.fd_array[i],&fdClients);
						this->deleteFd(fdClients.fd_array[i]);
						thing=0;
					}
					if(pfunc!=NULL)
					{
						if(pfunc(thing,num,sRec,pget,pneed,*this))
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
bool ServerTcpIp::disconnectSocket(SOCKET clisock)
{
    for(unsigned int i=0;i<fdClients.fd_count;i++)
        if(fdClients.fd_array[i]==clisock)
            FD_CLR(fdClients.fd_array[i],&fdClients);
    closesocket(clisock);
    return this->deleteFd(clisock);
}
bool ServerTcpIp::updateSocket(SOCKET* array,int* pcount,int arrayLen)
{
	if(fdNumNow!=0)
		*pcount=fdNumNow;
	else
		return false;
	for(int i=0;i<fdNumNow&&i<arrayLen;i++)
		array[i]=pfdn[i];
	return true;		
}
bool ServerTcpIp::findSocket(int cliSoc)
{
	for(int i=0;i<fdNumNow;i++)
		if(pfdn[i]==cliSoc)
			return true;
	return false;
}
char* ServerTcpIp::getHostIp()
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
char* ServerTcpIp::getHostName()
{
	char name[30]={0};
	gethostname(name,30);
	memcpy(hostname,name,30);
	return hostname;
}
char* ServerTcpIp::getPeerIp(SOCKET cliSoc,int* pcliPort)
{
	SOCKADDR_IN cliAddr={0};
	int len=sizeof(cliAddr);
	if(-1==getpeername(cliSoc,(SOCKADDR*)&cliAddr,&len))
		return NULL;
	*pcliPort=cliAddr.sin_port;
	return inet_ntoa(cliAddr.sin_addr); 
}
HttpServer::HttpServer(unsigned port,bool debug):ServerTcpIp(port)
{
	getText=NULL;
	array=NULL;
	array=(RouteFuntion*)malloc(sizeof(RouteFuntion)*20);
	if(array==NULL)
		this->error="route wrong";
	else
		memset(array,0,sizeof(RouteFuntion)*20);
	now=0;
	max=20;
	isDebug=debug;
	isLongCon=true;
	isFork=false;
	textLen=0;
	clientIn=NULL;
	clientOut=NULL;
	pnowRoute=NULL;
}
HttpServer::~HttpServer()
{
	if(array!=NULL)
		free(array);
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
bool HttpServer::loadStatic(const char* route,const char* staticPath)
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
	array[now].type=STATIC;
	array[now].ask=GET;
	strcpy(array[now].route,route);
	array[now].path=staticPath;
	array[now].pfunc=loadFile;
	now++;
	return true;
}
bool HttpServer::get(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
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
	array[now].ask=GET;
	strcpy(array[now].route,route);
	array[now].pfunc=pfunc;
	now++;
	return true;	
}
bool HttpServer::post(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
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
	array[now].ask=POST;
	strcpy(array[now].route,route);
	array[now].pfunc=pfunc;
	now++;
	return true;	
}
bool HttpServer::all(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
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
	array[now].ask=ALL;
	strcpy(array[now].route,route);
	array[now].pfunc=pfunc;
	now++;
	return true;	
}
void HttpServer::run(unsigned int memory,unsigned int recBufLenChar,const char* defaultFile)
{
	char* get=(char*)malloc(sizeof(char)*recBufLenChar);
	char* sen=(char*)malloc(sizeof(char)*memory*1024*1024);
	if(sen==NULL||get==NULL)
	{
		this->error="malloc get and sen wrong";
		return;
	}
	memset(get,0,sizeof(char)*recBufLenChar);
	memset(sen,0,sizeof(char)*memory*1024*1024);
	if(false==this->bondhost())
	{
		this->error="bound wrong";
		return;
	}
	if(false==this->setlisten())
	{
		this->error="set listen wrong";
		return;
	}
	this->getText=get;
	if(isDebug)
		printf("server is ok\n");
	if(isFork==false)
		while(1)
			this->epollHttp(get,recBufLenChar,memory,sen,defaultFile);
	else
		while(1)
			this->forkHttp(get,recBufLenChar,memory,sen,defaultFile);
	free(sen);
	free(get);
}
bool HttpServer::clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
{
	if(clientIn!=NULL)
		return false;
	clientIn=pfunc;
}
bool HttpServer::clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
{
	if(clientOut!=NULL)
		return false;
	clientOut=pfunc;
}
int HttpServer::httpSend(int num,void* buffer,int sendLen)
{
	return this->sendSocket(num,buffer,sendLen);
}
int HttpServer::httpRecv(int num,void* buffer,int bufferLen)
{
	return this->receiveSocket(num,buffer,bufferLen);
}
void HttpServer::changeSetting(bool debug,bool isLongCon,bool isForkModel)
{
	this->isDebug=debug;
	this->isLongCon=isLongCon;
	this->isFork=isForkModel;
}
int HttpServer::func(int num,void* pget,void* sen,unsigned int senLen,const char* defaultFile,HttpServer& server)
{
	static DealHttp http;
	AskType type=GET;
	int len=0,flag=2;
	char ask[200]={0};
	sscanf((char*)pget,"%100s",ask);
	if(strstr(ask,"GET")!=NULL)
	{
		http.getAskRoute(pget,"GET",ask,200);
		if(isDebug)
			printf("Get url:%s\n",ask);
		type=GET;
	}
	else if(strstr(ask,"POST")!=NULL)
	{
		http.getAskRoute(pget,"POST",ask,200);
		if(isDebug)
			printf("POST url:%s\n",ask);
		type=POST;
	}
	else if(strstr(ask,"PUT")!=NULL)
	{
		http.getAskRoute(pget,"PUT",ask,200);
		if(isDebug)
			printf("PUT url:%s\n",ask);
		type=PUT;
	}
	else if(strstr(ask,"DELETE")!=NULL)
	{
		http.getAskRoute(pget,"DELETE",ask,200);
		if(isDebug)
			printf("DELETE url:%s\n",ask);
		type=DELETETO;
	}
	void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&)=NULL;
	for(unsigned int i=0;i<now;i++)
	{
		if(array[i].type==ONEWAY&&(array[i].ask==type||array[i].ask==ALL))
		{
			if(strcmp(ask,array[i].route)==0)
			{
				pfunc=array[i].pfunc;
				pnowRoute=&array[i];
				break;
			}
		}
		else if(array[i].type==WILD&&(array[i].ask==type||array[i].ask==ALL))
		{
			if(strstr(ask,array[i].route)!=NULL)
			{
				pfunc=array[i].pfunc;
				pnowRoute=&array[i];
				break;						
			}
		}
		else if(array[i].type==STATIC&&type==GET)
		{
			if(strstr(ask,array[i].route)!=NULL)
			{
				pfunc=array[i].pfunc;
				sprintf((char*)sen,"%s",array[i].path);
				pnowRoute=&array[i];
				break;
			}
		}
	}
	if(pfunc!=NULL)
	{
		if(pfunc!=loadFile)
			pfunc(http,*this,num,sen,len);
		else
			pfunc(http,*this,senLen,sen,len);
	}
	else
	{
		if(isDebug)
			printf("http:%s\n",http.analysisHttpAsk(pget));
		if(http.analysisHttpAsk(pget)!=NULL)
		{
			strcpy(ask,http.analysisHttpAsk(pget));
			flag=http.autoAnalysisGet((char*)pget,(char*)sen,senLen*1024*1024,defaultFile,&len);
		}
		if(flag==2)
		{
			LogSystem::recordFileError(ask);
			if(isDebug)
				printf("404 get %s wrong\n",ask);
		}
	}
	if(len==0)
		http.createSendMsg(DealHttp::NOFOUND,(char*)sen,senLen*1024*1024,NULL,&len);
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
void HttpServer::epollHttp(void* pget,int len,unsigned int senLen,void* pneed,const char* defaultFile)
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
					{
						this->textLen=sRec;
						func(fdClients.fd_array[i],pget,pneed,senLen,defaultFile,*this);
						if(isLongCon==false)
						{
					  		this->deleteFd(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							closesocket(fdClients.fd_array[i]);						
						}
					}
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
void HttpServer::forkHttp(void* pget,int len,unsigned int senLen,void* pneed,const char* defaultFile)
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
					{
						this->textLen=sRec;
						func(fdClients.fd_array[i],pget,pneed,senLen,defaultFile,*this);
						if(isLongCon==false)
						{
					  		this->deleteFd(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							closesocket(fdClients.fd_array[i]);						
						}
					}
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
void HttpServer::loadFile(DealHttp& http,HttpServer& server,int senLen,void* sen,int& len)
{
	char ask[200]={0},buf[200]={0},temp[200]={0};
	http.getAskRoute(server.recText(),"GET",ask,200);
	HttpServer::RouteFuntion& route=*server.getNowRoute();
	http.getWildUrl(ask,route.route,temp,200);
	sprintf(buf,"GET %s%s HTTP/1.1",route.path,temp);
	if(2==http.autoAnalysisGet(buf,(char*)sen,senLen*1024*1024,NULL,&len))
	{
		LogSystem::recordFileError(ask);
		printf("404 get %s wrong\n",buf);
	}
}
ClientTcpIp::ClientTcpIp(const char* hostIp,int port)
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
ClientTcpIp::~ClientTcpIp()
{
	free(hostip);
	free(hostname);
	closesocket(sock);
	WSACleanup();
}
void ClientTcpIp::addHostIp(const char* ip,unsigned short port)
{
	if(ip==NULL)
		return;
	strcpy(this->ip,ip);
	addrC.sin_addr.S_un.S_addr=inet_addr(ip);
	if(port!=0)
		addrC.sin_port=htons(port);
}
bool ClientTcpIp::tryConnect()
{
	if(connect(sock,(SOCKADDR*)&addrC,sizeof(sockaddr))==SOCKET_ERROR)
		return false;
	return true;
}
bool ClientTcpIp::disconnectHost()
{
	closesocket(sock);
	sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<=0)
		return false;
	return true;
}
char* ClientTcpIp::getSelfIp()
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
char* ClientTcpIp::getSelfName()
{
	char name[30]={0};
	gethostname(name,30);
	memcpy(hostname,name,30);
	return hostname;
}
bool ClientTcpIp::getDnsIp(const char* name,char* ip,unsigned int ipMaxLen)
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
	if(strlen(inet_ntoa(addr))>=ipMaxLen)
		return false;
	strcpy(ip,inet_ntoa(addr));
	return true;
} 
