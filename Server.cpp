#include<iostream>
#include<cstdlib>
#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<pthread.h>
#include<queue>
#include</usr/include/mysql/mysql.h>
#include<openssl/ssl.h>
#include<openssl/err.h>
using namespace std;
/********************************
	author:chenxuan
	date:2021.8.1
	funtion:this file is all class and zhushi
*********************************/
class ServerTcpIp{
private:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now;
	int fd_count;//sum of clients in fd_set
	int last_count;//last fd_count
	int epfd;//file descriptor to ctrl epoll
	char* hostip;//host IP 
	char* hostname;//host name
	int sock;//file descriptor of host;
	int sockC;//file descriptor to sign every client;
	epoll_event nowEvent;//a temp event to get event
	epoll_event* pevent;//all the event
	sockaddr_in addr;//IPv4 of host;
	sockaddr_in client;//IPv4 of client;
	fd_set  fdClients;//file descriptor
protected:
    int* pfdn;//pointer if file descriptor
    int fdNumNow;//num of fd now
    int fdMax;//fd max num
    bool addFd(int addsoc)//add file des criptor
    {
        bool flag=false;
        for(int i=0;i<fdNumNow;i++)
        {
            if(pfdn[i]==0)
            {
                pfdn[i]=addsoc;
                flag=true;//has free room
                break;
            }
        }
        if(flag==false)//no free room
        {
            if(fdNumNow>=fdMax)
            {
                pfdn=(int*)realloc(pfdn,sizeof(int)*(fdMax+32));//try to realloc
                if(pfdn==NULL)
                	return false;
                fdMax+=31;
            }
            pfdn[fdNumNow]=addsoc;
            fdNumNow++;
        }
        return true;
    }
    bool deleteFd(int clisoc)//delete
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
	ServerTcpIp(unsigned short port=5200,int epollNum=1,int wait=5)
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
	~ServerTcpIp()//clean server
	{
		close(sock);
		close(sockC);
		close(epfd);
		free(hostip);
		free(hostname);
		free(pevent);
        free(pfdn);
	}
	bool bondhost()//bond myself first
	{
		if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
			return false;
		return true;
	}
	bool setlisten()//set listem to accept second
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
	int acceptClient()//wait until success model one
	{
		sockC=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
		return sockC;
	}
	bool acceptClients(int* pcliNum)//model two
	{
		*pcliNum=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
		return this->addFd(*pcliNum);
	}
	inline int receiveOne(void* pget,int len)//model one
	{
		return recv(sockC,(char*)pget,len,0);
	}
	inline int receiveSocket(int clisoc,void* pget,int len)
	{
		return recv(clisoc,(char*)pget,len,0);
	}
	inline int sendClientOne(const void* psen,int len)//model one
	{
		return send(sockC,(char*)psen,len,0);
	}
	void sendEverySocket(void* psen,int len)
    {
        for(int i=0;i<fdNumNow;i++)
            if(pfdn[i]!=0)
                send(pfdn[i],psen,len,0);
    }
	inline int sendSocket(int socCli,const void* psen,int len)//send by socket
	{
		return send(socCli,(char*)psen,len,0);
	}
	bool selectModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int,void* ,void*,ServerTcpIp& ))
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
	bool updateSocket(int* array,int* pcount)//get epoll array
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow;i++)
			array[i]=pfdn[i];
		return true;
	}
	bool findSocket(int cliSoc)//find if socket is connect
	{
		for(int i=0;i<fdNumNow;i++)
		{
			if(pfdn[i]==cliSoc)
				return true;
		}
		return false;
	}
	char* getHostName()//get self name
	{
		char name[300]={0};
		gethostname(name,300);
		memcpy(hostname,name,300);
		return hostname;
	}
	char* getHostIp()//get self ip
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
	char* getPeerIp(int cliSoc,int* pcliPort)//get ip and port by socket
	{
		sockaddr_in cliAddr={0};
		int len=sizeof(cliAddr);
		if(-1==getpeername(cliSoc,(sockaddr*)&cliAddr,(socklen_t*)&len))
			return NULL;
		*pcliPort=cliAddr.sin_port;
		return inet_ntoa(cliAddr.sin_addr); 
	}
	bool epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIp& ))
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
	bool disconnectSocket(int clisock)//disconnect from socket
    {
        close(clisock);
        return this->deleteFd(clisock);
    }
};
/********************************
	author:chenxuan
	date:2021/8/10
	funtion:class for client linux
*********************************/
class ClientTcpIp{
private:
	int sock;//myself
	sockaddr_in addrC;//server information
	char ip[100];//server Ip
	char* hostip;//host ip
	char* hostname;//host name
	char selfIp[100];
	SSL* ssl;
	SSL_CTX* ctx;
public:
	ClientTcpIp(const char* hostIp,unsigned short port)
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
		ssl=NULL;
		ctx=NULL;
	}
	~ClientTcpIp()
	{
		if(ssl!=NULL)
		{
			SSL_shutdown(ssl);
			SSL_free(ssl);	
		}
		if(ctx!=NULL)
			SSL_CTX_free(ctx);
		free(hostip);
		free(hostname);
		close(sock);
	}
	void addHostIp(const char* ip)
	{
		if(ip==NULL)
			return;
		strcpy(this->ip,ip);
		addrC.sin_addr.s_addr=inet_addr(ip);
	}
	bool tryConnect()
	{
		if(connect(sock,(sockaddr*)&addrC,sizeof(sockaddr))==-1)
			return false;
		return true;
	}
	inline int receiveHost(void* prec,int len)
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendHost(const void* ps,int len)
	{
		return send(sock,(char*)ps,len,0);
	}
	bool disconnectHost()
	{
		close(sock);
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(sock==-1)
			return false;
		return true;
	}
	char* getSelfIp()
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
	char* getSelfName(char* hostname)
	{
		char name[300]={0};
		gethostname(name,300);
		memcpy(hostname,name,strlen(name));
		return hostname;
	}
	static bool getDnsIp(const char* name,char* ip)
	{
		hostent* phost=gethostbyname(name);
		if(phost==NULL)
			return false;
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.s_addr,p,phost->h_length);
		strcpy(ip,inet_ntoa(addr));
		return true;
	}
	bool sslInit()
	{
		const SSL_METHOD* meth=SSLv23_client_method();
		if(meth==NULL)
			return false;
		ctx=SSL_CTX_new(meth);
		if(ctx==NULL)
			return false;
		ssl=SSL_new(ctx);
		if(NULL==ssl)
			return false;
		SSL_set_fd(ssl,sock);
		int ret=SSL_connect(ssl);
		if(ret==-1)
			return false;
		return true;
	}
	inline int sendhostSSL(const void* psen,int len)
	{
		return SSL_write(ssl,psen,len);
	}
	inline int receiveHostSSL(void* buffer,int len)
	{
		return SSL_read(ssl,buffer,len);
	}
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
		UNKNOWN=0,HTML=1,EXE=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP7=7
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
				"Connection: keep-alive\r\n");
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
		}
	}
	bool createSendMsg(FileKind kind,char* pask,const char* pfile,int* plong)
	{
		int temp=0;
		int len=0,noUse=0;
		if(kind==NOFOUND)
		{
			this->createTop(kind,pask,&temp,len);
			*plong=len+temp+1;
			return true;
		}
		len=this->getFileLen(pfile);
		if(len==0)
			return false;
		this->createTop(kind,pask,&temp,len);
		this->findFileMsg(pfile,&noUse,pask+temp);
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
	    else 
	        if(false==this->createSendMsg(UNKNOWN,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;
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
		while(*(ptemp++)!='\n'&&i<maxLineLen)
			line[i++]=*ptemp;
		line[i-1]=0;
		return line;
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
	date:2021/9/5
	funtion:class for deal Mysql ask
*********************************/
class MySql{
private:
    MYSQL* mysql;
    MYSQL one;
    MYSQL_RES* results;
    MySql(MySql&);
    MySql& operator==(MySql&);
public:
    MySql(const char* ipOrHostName,const char* user,const char* passwd,const char* dataBaseName)
    {
        this->results=NULL;
        this->mysql=mysql_init(NULL);
        if(NULL==mysql_real_connect(this->mysql,ipOrHostName,user,passwd,dataBaseName,0,NULL,0))
        {
            this->~MySql();
            throw;
        }
    }
    ~MySql()
    {
        if(this->results!=NULL)
            mysql_free_result(this->results);
        mysql_close(mysql);
    }
    int MySqlSelectQuery(const char* sql)
    {
        if(this->results!=NULL)
            mysql_free_result(this->results);
        int temp=mysql_query(this->mysql,sql);
        if(temp!=0)
            return 0;
        this->results=mysql_store_result(mysql);
        if(results==NULL)
            return 0;
        return mysql_num_fields(this->results);
    }
    bool MySqlOtherQuery(const char* sql)
    {
        int temp=mysql_query(this->mysql,sql);
        if(temp!=0)
            return false;
        return true;
    }
    char** MySqlGetResultRow()
    {
        if(this->results==NULL)
            return NULL;
        return mysql_fetch_row(results);
    }
    char* GetSqlLastError(char* errorSay)
    {
    	if(mysql_error(this->mysql)!=NULL)
			strcpy(errorSay,mysql_error(this->mysql));
		return errorSay;
	}
};
/********************************
	author:chenxuan
	date:2021/8/10
	funtion:class to deal ddos
*********************************/
class DealAttack{
public:
	struct CliLog{
		int socketCli;
		int time;
		char ip[20];
	};
	static bool dealAttack(int isUpdate,int socketCli,int maxTime)//check if accket
	{
		static CliLog cli[41];
		if(isUpdate==1)
		{
			cli[socketCli%41].socketCli=socketCli;
			cli[socketCli%41].time=1;
			return true;
		}
		else if(isUpdate==2)
		{
			cli[socketCli%41].time++;
			if(cli[socketCli%41].time>maxTime)
				return false;
			return true;
		}
		else if(isUpdate==0)
		{
			cli[socketCli%41].socketCli=0;
			cli[socketCli%41].time=0;
			return true;
		}
		return true;
	}
	static bool attackLog(int port,const char* ip,const char* pfileName)//log accket
	{
		time_t temp=time(NULL);
		struct tm* pt=localtime(&temp);
		FILE* fp=fopen(pfileName,"a+");
		if(fp==NULL)
			if((fp=fopen(pfileName,"w+"))==NULL)		
				return false;
			else
				fprintf(fp,"server attacked log\n");
		fprintf(fp,"%d year%d month%d day%d hour%d min%d sec:",pt->tm_year+1900,pt->tm_mon+1,pt->tm_mday,pt->tm_hour,pt->tm_min,pt->tm_sec);
		fprintf(fp,"%s:%d port attack server\n",ip,port);
		fclose(fp);
		return true;
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
};
/********************************
	author:chenxuan
	date:2021/8/10
	funtion:connect servertcpip and thread pool
*********************************/
class ServerTcpIpThreadPool{
private:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now; 
	int fd_count;//sum of clients in fd_set
	int last_count;//last fd_count
	int epfd;//file descriptor to ctrl epoll
	char* hostip;//host IP 
	char* hostname;//host name
	int sock;//file descriptor of host;
	int sockC;//file descriptor to sign every client;
	epoll_event nowEvent;//a temp event to get event
	epoll_event* pevent;//all the event
	sockaddr_in addr;//IPv4 of host;
	sockaddr_in client;//IPv4 of client;
	fd_set  fdClients;//file descriptor
	ThreadPool* pool;
	pthread_mutex_t mutex;
protected:
    int* pfdn;//pointer if file descriptor
    int fdNumNow;//num of fd now
    int fdMax;//fd max num
    bool addFd(int addsoc)//add file des criptor
    {
        bool flag=false;
        for(int i=0;i<fdNumNow;i++)
        {
            if(pfdn[i]==0)
            {
                pfdn[i]=addsoc;
                flag=true;//has free room
                break;
            }
        }
        if(flag==false)//no free room
        {
            if(fdNumNow>=fdMax)
            {
                pfdn=(int*)realloc(pfdn,sizeof(int)*fdMax+32);//try to realloc
                if(pfdn==NULL)
                	return false;
                fdMax+=10;
            }
            pfdn[fdNumNow]=addsoc;
            fdNumNow++;
        }
        return true;
    }
    bool deleteFd(int clisoc)//delete
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
	ServerTcpIpThreadPool(unsigned short port=5200,int epollNum=1,int wait=5,unsigned int threadNum=0)
	{
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
		if(threadNum>0)
		{	
			pool=new ThreadPool(threadNum);
			if(pool==NULL)
				throw NULL;
		}
		pthread_mutex_init(&mutex,NULL);
	}
	~ServerTcpIpThreadPool()//clean server
	{
		close(sock);
		close(sockC);
		close(epfd);
		free(hostip);
		free(hostname);
		free(pevent);
        free(pfdn);
        free(pool);
        pthread_mutex_destroy(&mutex);
	}
	void mutexLock()
	{
		pthread_mutex_lock(&mutex);
	}
	void mutexUnlock()
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
	bool bondhost()//bond myself first
	{
		if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
			return false;
		return true;
	}
	bool setlisten()//set listem to accept second
	{
		if(listen(sock,backwait)==-1)
			return false;
		FD_SET(sock,&fdClients);
		nowEvent.events=EPOLLIN;
		nowEvent.data.fd=sock;
		epoll_ctl(epfd,EPOLL_CTL_ADD,sock,&nowEvent);
		fd_count++;
		return true;
	}
	bool acceptClient()//wait until success model one
	{
		sockC=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
		return true;
	}
	bool acceptClients(int* pcliSoc)//model two
	{
		*pcliSoc=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
		return this->addFd(*pcliSoc);
	}
	inline int receiveOne(void* pget,int len)//model one
	{
		return recv(sockC,(char*)pget,len,0);
	}
	inline int receiveSocket(int clisoc,void* pget,int len)
	{
		return recv(clisoc,(char*)pget,len,0);
	}
	inline int sendClientOne(const void* ps,int len)//model one
	{
		return send(sockC,(char*)ps,len,0);
	}
	bool selectModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int,void* ,void*,ServerTcpIpThreadPool& ))
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
	bool updateSocket(int* p,int* pcount)//get epoll array
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow;i++)
			p[i]=pfdn[i];
		return true;
	}
    void sendEverySocket(void* ps,int len)
    {
        for(int i=0;i<fdNumNow;i++)
            if(pfdn[i]!=0)
                send(pfdn[i],ps,len,0);
    }
	inline int sendSocket(int socCli,const void* ps,int len)
	{
		return send(socCli,(char*)ps,len,0);
	}
	bool findSocket(int cliSoc)
	{
		for(int i=0;i<fdNumNow;i++)
		{
			if(pfdn[i]==cliSoc)
				return true;
		}
		return false;
	}
	char* getHostName()
	{
		char name[300]={0};
		gethostname(name,300);
		memcpy(hostname,name,300);
		return hostname;
	}
	char* getHostIp()
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
	bool epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIpThreadPool& ))
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
	char* getPeerIp(int cliSoc,int* pcliPort)
	{
		sockaddr_in cliAddr={0};
		int len=sizeof(cliAddr);
		if(-1==getpeername(cliSoc,(sockaddr*)&cliAddr,(socklen_t*)&len))
			return NULL;
		*pcliPort=cliAddr.sin_port;
		return inet_ntoa(cliAddr.sin_addr); 
	}
	bool disconnectSocket(int clisock)
    {
        close(clisock);
        return this->deleteFd(clisock);
    }
    void threadModel(void* pneed,void* (*pfunc)(void*))
    {
    	while(1)
    	{
			sockaddr_in newaddr={0};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
			if(newClient==-1)
				continue;
			this->addFd(newClient);
			ServerTcpIpThreadPool::ArgvSer argv{*this,newClient,pneed};
			ThreadPool::Task task={pfunc,&argv};
			pool->addTask(task);
		}
	}
	bool epollThread(int* pthing,int* pnum,void* pget,int len,void* pneed,void* (*pfunc)(void*))
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
				*pthing=1;
				*pnum=newClient;
				strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
				ServerTcpIpThreadPool::ArgvSerEpoll argv={*this,*pnum,*pthing,0,pneed,pget};
				ThreadPool::Task task={pfunc,&argv};
				if(pfunc!=NULL)
					pool->addTask(task);
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
					ServerTcpIpThreadPool::ArgvSerEpoll argv={*this,*pnum,*pthing,getNum,pneed,pget};
					ThreadPool::Task task={pfunc,&argv};
					if(pfunc!=NULL)
						pool->addTask(task);
				}
			}
		}
		return true;
	}
	inline bool threadDeleteSoc(int clisoc)
	{
		close(clisoc);
		return this->deleteFd(clisoc);
	}
};
/********************************
	author:chenxuan
	date:2021/9/4
	funtion:the class ti ctrl file get and len 
*********************************/
class FileGet{
private:
	char* pbuffer;
public:
	FileGet()
	{
		pbuffer=NULL;
	}
	~FileGet()
	{
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
	}
	int getFileLen(const char* fileName)
	{
		int len=0;
		FILE* fp=fopen(fileName,"rb");
		if(fp==NULL)
			return -1;
		fseek(fp,0,SEEK_END);
		len=ftell(fp);
		fclose(fp);
		return len;
	}
	bool getFileMsg(const char* fileName,char* buffer)
	{
		int i=0,len=0;
		len=this->getFileLen(fileName);
		FILE* fp=fopen(fileName,"rb");
		if(fp==NULL)
			return false;
		for(i=0;i<len;i++)
			buffer[i]=fgetc(fp);
		buffer[i+1]=0;
		fclose(fp);
		return true;
	}
	bool fileStrstr(const char* fileName,const char* strFind)
	{
		int len=0;
		char* pstr=NULL;
		len=this->getFileLen(fileName);
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
		FILE* fp=fopen(fileName,"r");
		if(fp==NULL)
			return false;
		pbuffer=(char*)malloc(sizeof(char)*(len+10));
		char* ptemp=pbuffer;
		if(pbuffer==NULL)
			return false;
		memset(pbuffer,0,sizeof(char)*(len+5));
		if(false==this->getFileMsg(fileName,pbuffer))
			return false;
		while((*ptemp<65||*ptemp>122)&&ptemp<pbuffer+sizeof(char)*len)
			ptemp++;
		pstr=strstr(ptemp,strFind);
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
		fclose(fp);
		if(pstr!=NULL)
			return true;
		else
			return false;
		return false;
	}
};
/********************************
	author:chenxuan
	date:2021/9/8
	funtion:a struct temp but no use
*********************************/
struct CliMsg{ 
	char hao[20];
	char mi[20];
	int flag;
	char chat[100];
};
int funcTwo(int thing,int num,int,void* pget,void* sen,ServerTcpIp& server)//main deal func
{
	char ask[200]={0},*pask=NULL;
	CliMsg cli;
	DealHttp http;
	int len=0;
	int port=0;
	int flag=0;
	if(sen==NULL)
		return -1;
	memset(sen,0,sizeof(char)*10000000);
	if(false==DealAttack::dealAttack(thing,num,200))
	{
		DealAttack::attackLog(port,server.getPeerIp(num,&port),"./rec/attackLog.txt");
		server.disconnectSocket(num);
		return 0;
	}
	if(thing==0)
		printf("%d is out\n",num);
	if(thing==1)
		printf("%s in %d\n",(char*)pget,num);
	if(thing==2)
	{
		if(http.getKeyLine(pget,"Accept-Language",ask,200)!=NULL)
			printf("\n%s\n",ask);
		if(false==http.cutLineAsk((char*)pget,"GET"))
			return 0;
		printf("ask:%s\n",(char*)pget);
		printf("http:%s\n",http.analysisHttpAsk(pget));
		strcpy(ask,http.analysisHttpAsk(pget));
		flag=http.autoAnalysisGet((char*)pget,(char*)sen,"./index.html",&len);
		if(0==flag)
			printf("some thing wrong %s\n",(char*)sen);
		else if(flag==1)
			printf("create auto success\n");
        else if(flag==2)
        {
            FILE* fp=fopen("wrong.txt","a+");
            if(fp==NULL)
                fp=fopen("wrong.txt","w+");
            if(fp==NULL)
                return 0;
            fprintf(fp,"can not open file %s\n",ask);
            printf("cannot open file %s\n",ask);
            fclose(fp);
        }
		
		if(false==server.sendSocket(num,sen,len))
			printf("send wrong\n");
		else
			printf("send success\n");
	}
	return 0;
} 
/********************************
	author:chenxuan
	date:2021/9/8
	funtion:the main funtion to deal message
	parameter:thing stand what happen,num is socket,len is get len,
	return:return 0 stand continue,other stop
*********************************/
int funcThree(int thing,int num,int,void* pget,void* sen,ServerTcpIp& server)//main deal func
{
	char ask[200]={0},*pask=NULL;
	DealHttp http;
	int len=0;
	int port=0;
	int flag=0;
	if(sen==NULL)
		return -1;
	memset(sen,0,sizeof(char)*10000000);
	if(false==DealAttack::dealAttack(thing,num,200))
	{
		DealAttack::attackLog(port,server.getPeerIp(num,&port),"./rec/attackLog.txt");
		server.disconnectSocket(num);
		return 0;
	}
	if(thing==0)
		printf("%d is out\n",num);
	if(thing==1)
		printf("%s in %d\n",(char*)pget,num);
	if(thing==2)
	{
		if(http.getKeyLine(pget,"Accept-Language",ask,200)!=NULL)
			printf("\n%s\n",ask);
		if(false==http.cutLineAsk((char*)pget,"GET"))
			return 0;
		printf("ask:%s\n",(char*)pget);
		printf("http:%s\n",http.analysisHttpAsk(pget));
		strcpy(ask,http.analysisHttpAsk(pget));
		flag=http.autoAnalysisGet((char*)pget,(char*)sen,"./index.html",&len);
		if(0==flag)
			printf("some thing wrong %s\n",(char*)sen);
		else if(flag==1)
			printf("create auto success\n");
        else if(flag==2)
        {
            FILE* fp=fopen("wrong.txt","a+");
            if(fp==NULL)
                fp=fopen("wrong.txt","w+");
            if(fp==NULL)
                return 0;
            fprintf(fp,"can not open file %s\n",ask);
            printf("cannot open file %s\n",ask);
            fclose(fp);
        }
		if(false==server.sendSocket(num,sen,len))
			printf("send erong\n");
		else
			printf("send success\n");
		return 0;
	}
	return 0;
}
/********************************
	author:chenxuan
	date:2021/9/8
	funtion:select model
	parameter:void
	return:void
*********************************/
void selectTry()
{
	ServerTcpIp server(5201);
	int thing=0,num=0;
	char get[2048]={0};
	char* sen=(char*)malloc(sizeof(char)*10000000);
	if(sen==NULL)
		printf("memory wrong\n");
	if(false==server.bondhost())
	{
		printf("bound wrong\n");
		return;
	}
	if(false==server.setlisten())
		exit(0);
	printf("server ip is:%s\nthe server is ok\n",server.getHostIp());
	while(1)
		server.selectModel(&thing,&num,get,2048,sen,funcThree);
	free(sen);
}
/********************************
	author:chenxuan
	date:2021/9/8
	funtion:epoll model
	parameter:void
	return:void
*********************************/
void serverHttp()
{
	int pid=0;
//	if((pid=fork())!=0)
//	{
//		printf("pid=%d",pid);
//		return;
//	}
	ServerTcpIp server(5201,100);
	int thing=0,num=0;
	char get[2048]={0};
	char* sen=(char*)malloc(sizeof(char)*10000000);
	if(sen==NULL)
		printf("memory wrong\n");
	if(false==server.bondhost())
	{
		printf("bound wrong\n");
		return;
	}
	if(false==server.setlisten())
		exit(0);
	printf("server ip is:%s\nthe server is ok\n",server.getHostIp());
	while(1)
		if(false==server.epollModel(&thing,&num,get,2048,sen,funcTwo))
			break;
	free(sen);
}
/********************************
	author:chenxuan
	date:2021/9/9
	funtion:thank you for watching
*********************************/
int main(int argc, char** argv) 
{
	serverHttp();
	return 0;
}
