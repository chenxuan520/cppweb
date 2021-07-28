#include<iostream>
#include<cstdlib>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include</usr/include/mysql/mysql.h>
#include<netdb.h>
#include<pthread.h>
#include<queue>
using namespace std;
class ServerTcpIp{
private:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now; 
	int max;//the most clients;
	int fd_count;//sum of clients in fd_set
	int epfd;//file descriptor to ctrl epoll
	char* hostip;//host IP 
	char* hostname;//host name
	int sock;//file descriptor of host;
	int* psockClients;//client[];
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
                pfdn=(int*)realloc(pfdn,sizeof(int)*fdMax+32);
                if(pfdn==NULL)
                    exit(0);
                fdMax+=10;
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
	ServerTcpIp(unsigned short port=5200,int epollNum=0,int wait=5,int maxClient=0)
	{
		sock=socket(AF_INET,SOCK_STREAM,0);//AF=addr family internet
		addr.sin_addr.s_addr=htonl(INADDR_ANY);//inaddr_any
		addr.sin_family=AF_INET;//af_intt IPv4
		addr.sin_port=htons(port);//host to net short
		fd_count=0;// select model
		sizeAddr=sizeof(sockaddr);
		backwait=wait;
		numClient=0;
		hostip=(char*)malloc(sizeof(char)*20);
		memset(hostip,0,sizeof(char)*20);
		hostname=(char*)malloc(sizeof(char)*30);
		memset(hostname,0,sizeof(char)*30);
		FD_ZERO(&fdClients);//clean fdClients;
		epfd=epoll_create(epollNum);
		if((pevent=(epoll_event*)malloc(512*sizeof(epoll_event)))==NULL)
			exit(0);
		memset(pevent,0,sizeof(epoll_event)*512);
		memset(&nowEvent,0,sizeof(epoll_event));
        pfdn=(int*)malloc(sizeof(int)*64);
        if(pfdn==NULL)
            exit(0);
        memset(pfdn,0,sizeof(int)*64);
        fdNumNow=0;
        fdMax=64;
		if(maxClient<=0||maxClient>100)
			psockClients=NULL;
		else 
		{
			psockClients=(int*)malloc(sizeof(int)*maxClient);
			if(psockClients==NULL)
				exit(0);
			max=maxClient;
		}
	}
	~ServerTcpIp()//clean server
	{
		if(psockClients!=NULL)
			free(psockClients);
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
		fd_count++;
		return true;
	}
	bool acceptClient()//wait until success model one
	{
		sockC=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
		return true;
	}
	bool acceptClientsModelTwo(int cliNum)//model two
	{
		if(cliNum<max)
		{
			psockClients[cliNum]=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
			numClient++;
		}	
		else
			return false;
		return true;
	}
	inline int receiveMystl(void* pget,int len)//model one
	{
		return recv(sockC,(char*)pget,len,0);
	}
	int receiveSMystlModelTwo(void* prec,int cliNum,int len)//model two
	{
		int temp=recv(psockClients[cliNum],(char*)prec,len,0);
		if(temp==-1)
			psockClients[cliNum]=0;
		return temp;
	}
	inline int sendClientMystl(const void* ps,int len)//model one
	{
		return send(sockC,(char*)ps,len,0);
	}
	inline int sendClientSMystlModelTwo(const void* ps,int cliNum,int len)//model two
	{
		return send(psockClients[cliNum],(char*)ps,len,0);
	}
	void sendClientsEveryoneMystlTwo(const void* ps,int len)//model two
	{
		for(int i=0;i<numClient;i++)
		{ 
			if(psockClients[i]==0)
				continue;
			send(psockClients[i],(char*)ps,len,0);
		}
	}
	bool selectModelMysql(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int,void* ,void*,ServerTcpIp& ))
	{//0 out,1 in,2 say
		fd_set temp=fdClients;
		int sign=select(0,&temp,NULL,NULL,NULL);
		if(sign>0)
		{
			for(int i=0;i<(int)fd_count;i++)
			{
				if(FD_ISSET(fdClients.fds_bits[i],&temp))
				{
					if(fdClients.fds_bits[i]==sock)
					{
						if(fd_count<FD_SETSIZE)
						{
							sockaddr_in newaddr={0};
							int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							fd_count++;
							for(int j=0;j<(int)fd_count;j++)
							{
								if(newClient==fdClients.fds_bits[j])
								{
									*pnum=j;
									break;
								}
							}
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
						int sRec=recv(fdClients.fds_bits[i],(char*)pget,len,0);
						*pnum=i;
						if(sRec>0)
						{
							*pthing=2;
						}
						if(sRec<=0)
						{
							close(fdClients.fds_bits[i]);
							this->deleteFd(fdClients.fds_bits[i]);
							FD_CLR(fdClients.fds_bits[i],&fdClients);
							fd_count--;
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
	inline int selectSendMystl(const void* ps,int cliNum,int len)
	{
		return send(fdClients.fds_bits[cliNum],(char*)ps,len,0);
	}
	void selectSendEveryoneMystl(void* ps,int len)
	{
		for(int i=0;i<fd_count;i++)
		{ 
			int clientSent=fdClients.fds_bits[i];
			if(clientSent!=0)
				send(fdClients.fds_bits[i],(char*)ps,len,0);
		}
	}
	bool updateSocketSelect(int* p,int* pcount)
	{
		if(fd_count!=0)
			*pcount=fd_count;
		else
			return false;
		for(int i=0;i<fd_count;i++)
			p[i]=fdClients.fds_bits[i];
		return true;
	}
	bool updateSocketEpoll(int* p,int* pcount)
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow;i++)
			p[i]=pfdn[i];
		return true;
	}
	int sendSocketMystlSelect(int toClient,const void* ps,int len)
	{
		for(int i=0;i<fd_count;i++)
			if(toClient==fdClients.fds_bits[i])
				return send(fdClients.fds_bits[i],(char*)ps,len,0);
		return -1;
	}
    void sendEverySocket(void* ps,int len)
    {
        for(int i=0;i<fdNumNow;i++)
            if(pfdn[i]!=0)
                send(pfdn[i],ps,len,0);
    }
	inline int sendSocketAllModel(int socCli,const void* ps,int len)
	{
		return send(socCli,(char*)ps,len,0);
	}
	int findSocketSelsct(int i)
	{
		if(fdClients.fds_bits[i]!=0)
			return fdClients.fds_bits[i];
		else
			return -1;
	}
	bool findSocketEpoll(int cliSoc)
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
		char name[30]={0};
		gethostname(name,30);
		memcpy(hostname,name,30);
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
		memset(hostip,0,sizeof(char)*20);
		memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return hostip;
	}
	bool epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIp& ))
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
	bool disconnectSocketEpoll(int clisock)
    {
        close(clisock);
        return this->deleteFd(clisock);
    }
};
class ClientTcpIp{
private:
	int sock;//myself
	sockaddr_in addrC;//server information
	char ip[100];//server Ip
	char* hostip;//host ip
	char* hostname;//host name
	char selfIp[30];
public:
	ClientTcpIp(const char* hostIp,unsigned short port)
	{
		memset(ip,0,100);
		memset(selfIp,0,30);
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
	}
	~ClientTcpIp()
	{
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
	inline int receiveMystl(void* prec,int len)
	{
		return recv(sock,(char*)prec,len,0);
	}
	int sendHostMystl(const void* ps,int len)
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
		char name[30]={0};
		gethostname(name,30);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.s_addr,p,phost->h_length);
		memset(selfIp,0,sizeof(char)*30);
		memcpy(selfIp,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return selfIp;
	}
	char* getHostName(char* hostname)
	{
		char name[30]={0};
		gethostname(name,30);
		memcpy(hostname,name,30);
		return hostname;
	}
};
class DealHttp{
private:
	char ask[256];
	char* pfind;
	char* pfile;
public:
	DealHttp()
	{
		for(int i=0;i<256;i++)
			ask[i]=0;
		pfind=NULL;
		pfile=NULL;
	}
	~DealHttp()
	{
		if(pfile!=NULL)
			free(pfile);
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
		return this->findBackString(pfind,needLen,ask);
	}
	inline char* findFirst(void* pask,const char* ptofind)
	{
		return strstr((char*)pask,ptofind);
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
			if((*pend>90&&*pend<97&&*pend!=95)||(*pend<48&&*pend!=46&&*pend!=47&&*pend!=45)||*pend>122||*pend==63)
				break;
			else
				pend++;
		for(char* pi=ptemp;pi<pend;pi++)
			word[i++]=*pi;
		word[i]=0;
		return word;
	}
	void createTop(int kind,char* ptop,int* topLen,int fileLen)//1:http 2:down 3:pic
	{
		switch (kind)
		{
			case 0:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case 1:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:text/html\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case 2:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:application/octet-stream\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case 3:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:image\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case 4:
				*topLen=sprintf(ptop,"HTTP/1.1 404 Not Fount\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n");
				break;
			case 5:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:text/css\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case 6:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:text/javascript\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
			case 7:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:application/x-7z-compressed\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
		}
	}
	bool createSendMsg(int kind,char* pask,const char* pfile,int* plong)
	{
		int temp=0;
		int len=0,noUse=0;
		switch (kind)
		{
		case 0:
			len=this->getFileLen(pfile);
			if(len==0)
				return false;
			this->createTop(0,pask,&temp,len);
			memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);
			break;
		case 1:
			len=this->getFileLen(pfile);
			if(len==0)
				return false;
			this->createTop(1,pask,&temp,len);
			memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);
			break;
		case 2:
			len=this->getFileLen(pfile);
			if(len==0)
				return false;
			this->createTop(2,pask,&temp,len);
			memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);
			break;
		case 3:
			len=this->getFileLen(pfile);
			if(len==0)
				return false;
			this->createTop(3,pask,&temp,len);
			memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);
			break;
		case 4:
			this->createTop(4,pask,&temp,len);
			break;
		case 5:
			len=this->getFileLen(pfile);
			if(len==0)
				return false;
			this->createTop(5,pask,&temp,len);
			memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);printf("yes create\n");
			break;
		case 6:
			len=this->getFileLen(pfile);
			if(len==0)
				return false;
			this->createTop(6,pask,&temp,len);
			memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);
			break;
		case 7:
			len=this->getFileLen(pfile);
			if(len==0)
				return false;
			this->createTop(7,pask,&temp,len);
			memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);
		default:
			break;
		}
		*plong=len+temp+10;
		return true;
	}
	char* findFileMsg(const char* pname,int* plen)
	{
		FILE* fp=fopen(pname,"rb+");
		int flen=0,i=0;
		if(fp==NULL)
			return NULL;
		fseek(fp,0,SEEK_END);
		flen=ftell(fp);
		if(pfile!=NULL)
			free(pfile);
		pfile=(char*)malloc(sizeof(char)*flen+10);
		if(pfile==NULL)
			return NULL;
		memset(pfile,0,sizeof(char)*flen+10);
		fseek(fp,0,SEEK_SET);
		for(i=0;i<flen;i++)
			pfile[i]=fgetc(fp);
		pfile[i]=0;
		*plen=flen;
		fclose(fp);
		return pfile;
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
	        if(false==this->createSendMsg(1,psend,pfirstFile,plen))
	        {
	            if(false==this->createSendMsg(4,psend,pfirstFile,plen))
	                return 0;
	            else 
	                return 2;
	        }
	    }
	    else if(strstr(ask,".html"))
	    {
	        if(false==this->createSendMsg(1,psend,ask,plen))
	            if(false==this->createSendMsg(4,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	    }
	    else if(strstr(ask,".exe"))
	    {
	        if(false==this->createSendMsg(2,psend,ask,plen))
	            if(false==this->createSendMsg(4,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	    }
	    else if(strstr(ask,".PNG")||strstr(ask,".jpg"))
	    {
	        if(false==this->createSendMsg(3,psend,ask,plen))
	            if(false==this->createSendMsg(4,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	    }
	    else if(strstr(ask,".css"))
	    {
	        if(false==this->createSendMsg(5,psend,ask,plen))
	            if(false==this->createSendMsg(4,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	    }
	    else if(strstr(ask,".js"))
	    {
	        if(false==this->createSendMsg(6,psend,ask,plen))
	            if(false==this->createSendMsg(4,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	    }
	    else 
	        if(false==this->createSendMsg(0,psend,ask,plen))
	            if(false==this->createSendMsg(4,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	    return 1;
	}
};
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
            cerr<<mysql_error(this->mysql)<<endl;
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
        {
            cerr<<mysql_error(this->mysql)<<endl;
            return 0;
        }
        this->results=mysql_store_result(mysql);
        if(results==NULL)
        {
            cerr<<mysql_error(this->mysql)<<endl;
            return 0;
        }
        return mysql_num_fields(this->results);
    }
    bool MySqlOtherQuery(const char* sql)
    {
        int temp=mysql_query(this->mysql,sql);
        if(temp!=0)
        {
            cerr<<mysql_error(this->mysql)<<endl;
            return false;
        }
        return true;
    }
    inline char** MySqlGetResultRow()
    {
        if(this->results==NULL)
            return NULL;
        return mysql_fetch_row(results);
    }
};
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
				fprintf(fp,"服务器被进攻日志\n");
		fprintf(fp,"%d年%d月%d日%d时%d分%d秒:",pt->tm_year+1900,pt->tm_mon+1,pt->tm_mday,pt->tm_hour,pt->tm_min,pt->tm_sec);
		fprintf(fp,"%s:%d 端口发起对服务器进攻\n",ip,port);
		fclose(fp);
		return true;
	}
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
			exit(0);
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
};
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
	if(sen==NULL)
		return -1;
	memset(sen,0,sizeof(char)*10000000);
	if(false==DealAttack::dealAttack(thing,num,200))
	{
		DealAttack::attackLog(port,server.getPeerIp(num,&port),"./rec/attackLog.txt");
		server.disconnectSocketEpoll(num);
		return 0;
	}
	if(thing==0)
		printf("%d is out\n",num);
	if(thing==1)
		printf("%s in %d\n",(char*)pget,num);
	if(thing==2)
	{
		if(false==http.cutLineAsk((char*)pget,"GET"))
			return 0;
		printf("ask:%s\n",(char*)pget);
		printf("http:%s\n",http.analysisHttpAsk(pget));
		strcpy(ask,http.analysisHttpAsk(pget));
		if(false==http.autoAnalysisGet((char*)pget,(char*)sen,"./rec/index.html",&len))
			printf("some thing wrong %s\n",(char*)sen);
		else
			printf("create auto success\n");
		if(false==server.sendSocketAllModel(num,sen,len))
			printf("send wrong\n");
		else
			printf("send success\n");
	}
	return 0;
}
void serverHttp()
{
	int pid=0;
	if((pid=fork())!=0)
	{
		printf("pid=%d",pid);
		return;
	}
	ServerTcpIp server(100,5201);
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
		if(false==server.epollModel(&thing,&num,get,2048,sen,funcTwo))
			break;
	free(sen);
}
int main(int argc, char** argv) 
{
	serverHttp();
	return 0;
}
