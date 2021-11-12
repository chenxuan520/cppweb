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
						sockaddr_in newaddr={0};
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
bool ServerTcpIp::epollModel(void* pget,int len,void* pneed,int (*pfunc)(Thing,int ,int ,void* ,void*,ServerTcpIp& ))
{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
	Thing thing=SAY;
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
	clientIn=NULL;
	clientOut=NULL;
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
int HttpServer::func(int num,void* pget,void* sen,const char* defaultFile,HttpServer& server)
{
	static DealHttp http;
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
		if(flag==2&&isDebug)
		{
			LogSystem::recordFileError(ask);
			printf("404 get %s wrong\n",ask);
		}
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
void HttpServer::epollHttp(void* pget,int len,void* pneed,const char* defaultFile)
{
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
			if(this->clientIn!=NULL)
			{
				strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
				clientIn(*this,newClient,pget,newaddr.sin_port);
			}
		}
		else
		{
			int getNum=recv(temp.data.fd,(char*)pget,len,0);
			if(getNum>0)
				func(temp.data.fd,pget,pneed,defaultFile,*this);
			else
			{
				if(this->clientOut!=NULL)
				{
					int port=0;
					strcpy((char*)pget,this->getPeerIp(temp.data.fd,&port));
					clientOut(*this,temp.data.fd,pget,port);
				}
                this->deleteFd(temp.data.fd);
				epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
				close(temp.data.fd);
			}
		}
	}
	return ;
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
	memset(hostip,0,sizeof(char)*50);
	hostname=(char*)malloc(sizeof(char)*50);
	memset(hostname,0,sizeof(char)*50);
	if(hostIp!=NULL)
		strcpy(ip,hostIp);
	sock=socket(AF_INET,SOCK_STREAM,0);
	if(hostIp!=NULL)
		addrC.sin_addr.s_addr=inet_addr(hostIp);
	addrC.sin_family=AF_INET;//af_intt IPv4
	addrC.sin_port=htons(port);
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
void ClientTcpIp::addHostIp(const char* ip)
{
	if(ip==NULL)
		return;
	strcpy(this->ip,ip);
	addrC.sin_addr.s_addr=inet_addr(ip);
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
