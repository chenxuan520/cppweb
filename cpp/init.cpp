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
	ServerTcpIp(int epollNum=0,unsigned short port=5200,int wait=5,int maxClient=0)
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
	bool receiveMystl(void* pget,int len)//model one
	{
		if(recv(sockC,(char*)pget,len,0)==-1)
			return false;
		return true;
	}
	bool receiveSMystlModelTwo(void* prec,int cliNum,int len)//model two
	{
		if(recv(psockClients[cliNum],(char*)prec,len,0)==-1)
			psockClients[cliNum]=0;
			return false;
		return true;
	}
	bool sendClientMystl(const void* ps,int len)//model one
	{
		if(send(sockC,(char*)ps,len,0)==-1)
			return false;
		return true;
	}
	bool sendClientSMystlModelTwo(const void* ps,int cliNum,int len)//model two
	{
		if(send(psockClients[cliNum],(char*)ps,len,0)==-1)
			return false;
		return true;
	}
	bool sendClientsEveryoneMystlTwo(const void* ps,int len)//model two
	{
		for(int i=0;i<numClient;i++)
		{ 
			if(psockClients[i]==0)
				continue;
			send(psockClients[i],(char*)ps,len,0);
		}
		return true;
	}
	bool selectModelMysql(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,void* ,void*,ServerTcpIp& ))
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
								if(pfunc(*pthing,*pnum,pget,pneed,*this))
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
							if(pfunc(*pthing,*pnum,pget,pneed,*this))
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
	bool selectSendMystl(const void* ps,int cliNum,int len)
	{
		int clientSent=fdClients.fds_bits[cliNum];
		if(send(fdClients.fds_bits[cliNum],(char*)ps,len,0)==-1)
			return false;
		return true;
	}
	bool selectSendEveryoneMystl(void* ps,int len)
	{
		for(int i=0;i<fd_count;i++)
		{ 
			int clientSent=fdClients.fds_bits[i];
			if(clientSent!=0)
				send(fdClients.fds_bits[i],(char*)ps,len,0);
		}
		return true;
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
	bool sendSocketMystlSelect(int toClient,const void* ps,int len)
	{
		for(int i=0;i<fd_count;i++)
		{
			if(toClient==fdClients.fds_bits[i])
			{
				send(fdClients.fds_bits[i],(char*)ps,len,0);
				return true;
			}
		}
		return false;
	}
    bool sendEverySocket(void* ps,int len)
    {
        for(int i=0;i<fdNumNow;i++)
        {
            if(pfdn[i]!=0)
                send(pfdn[i],ps,len,0);
        }
        return true;
    }
	bool sendSocketAll(int socCli,const void* ps,int len)
	{
		if(send(socCli,(char*)ps,len,0)==-1)
			return false;
		return true;
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
		char name[30]={0};
		gethostname(name,30);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.s_addr,p,phost->h_length);
		memset(hostip,0,sizeof(char)*20);
		memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return hostip;
	}
	bool epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,void* ,void*,ServerTcpIp& ))
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
					if(pfunc(*pthing,*pnum,pget,pneed,*this))
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
					if(pfunc(*pthing,*pnum,pget,pneed,*this))
						return false;
				}
			}
		}
		return true;
	}
};
class DealHttp{
private:
	char ask[200];
	char* pfind;
	char* pfile;
public:
	DealHttp()
	{
		for(int i=0;i<200;i++)
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
			if((*pend>90&&*pend<97&&*pend!=95)||(*pend<48&&*pend!=46&&*pend!=47)||*pend>122||*pend==63)
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
	bool autoAnalysisGet(const char* pask,char* psend,const char* pfirstFile,int* plen)
	{
		if(NULL==this->analysisHttpAsk((void*)pask))
			return false;
		if(strcmp(ask,"HTTP/1.1")==0)
		{
			if(false==this->createSendMsg(1,psend,pfirstFile,plen))
			{
				if(false==this->createSendMsg(4,psend,pfirstFile,plen))
					return false;
			}
		}
		else if(strstr(ask,".html"))
		{
			if(false==this->createSendMsg(1,psend,ask,plen))
				if(false==this->createSendMsg(4,psend,ask,plen))
					return false;
		}
		else if(strstr(ask,".exe"))
		{
			if(false==this->createSendMsg(2,psend,ask,plen))
				if(false==this->createSendMsg(4,psend,ask,plen))
					return false;
		}
		else if(strstr(ask,".PNG")||strstr(ask,".jpg"))
		{
			if(false==this->createSendMsg(3,psend,ask,plen))
				if(false==this->createSendMsg(4,psend,ask,plen))
					return false;
		}
		else 
			if(false==this->createSendMsg(0,psend,ask,plen))
				if(false==this->createSendMsg(4,psend,ask,plen))
					return false;
		return true;
	}
};
char* findBackString(char* ps,int len,char* word)//the same to last
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
		if((*pend>90&&*pend<97&&*pend!=95)||(*pend<48&&*pend!=46)||*pend>122||(*pend>57&&*pend<65))
			break;
		else
			pend++;
	for(char* pi=ptemp;pi<pend;pi++)
		word[i++]=*pi;
	word[i]=0;
	return word;
}
bool signIn(ServerTcpIp& server,DealHttp& http,void* sen,char* ask,int* plen)
{
	char* temp=NULL;
	char word[20]={0},portArray[20]={0},memoryArray[20]={0};
	int port=0,memory=0;
	FILE* fp=fopen("my.ini","r+");
	if(fp==NULL)
		fp=fopen("my.ini","w+");
	if(fp==NULL)
		return false;
    printf("open success %s\n",ask);
	if((temp=strstr(ask,"port"))==NULL)
		return  false;
	temp=http.findBackString(temp,4,word);
	if(sscanf(word,"%d",&port)!=1)
		printf("scanf int wrong\n");
	fprintf(fp,"%d\n",port);
	if((temp=strstr(ask,"Html"))==NULL)
		printf("scanf html wrong\n");
	memset(word,0,20);
	temp=http.findBackString(temp,4,word);
	fprintf(fp,"%s\n",word);
	if((temp=strstr(ask,"memory"))==NULL)
		printf("scanf memory wrong\n");
	memset(word,0,20);
	temp=http.findBackString(temp,6,word);
	if(sscanf(word,"%d",&memory)!=1)
		printf("get memory wrong\n");
	fprintf(fp,"%d\n",memory);
	fprintf(fp,"y\n");
	remove("rec/index.html");
    rename("rec/ini.html","rec/index.html");
	sprintf(portArray,"%d",port);
    sprintf(memoryArray,"%d",memory);
    if(false==http.createSendMsg(1,(char*)sen,"rec/index.html",plen))
        printf("create false");
    else
        printf("create success\n");    
    fclose(fp);
	return true;
}
int funcTwo(int thing,int num,void* pget,void* sen,ServerTcpIp& server)//main deal func
{
	char ask[100]={0},*pask=NULL;
	DealHttp http;
	int len=0;
	if(sen==NULL)
		return -1;
	memset(sen,0,sizeof(char)*1000000);
	if(thing==0)
	{
		printf("%d is out\n",num);
	}
	if(thing==1)
	{
		printf("%s in %d\n",(char*)pget,num);
	}
	if(thing==2)
	{
		if(false==http.cutLineAsk((char*)pget,"GET"))
			return 0;
		printf("ask:%s\n",(char*)pget);
		printf("http:%s\n",http.analysisHttpAsk(pget));
		strcpy(ask,http.analysisHttpAsk(pget));
		if(signIn(server,http,sen,(char*)pget,&len)==true)
			printf("init success\n");
		else
		{
			if(false==http.autoAnalysisGet((char*)pget,(char*)sen,"rec/index.html",&len))
				printf("some thing wrong %s\n",(char*)sen);
			else
				printf("create auto success\n");
		}
		if(false==server.sendSocketAll(num,sen,len))
			printf("send wrong\n");
		else
			printf("send success\n");
	}
	return 0;
}
void serverHttp()
{
	ServerTcpIp server(100,8888);
	int thing=0,num=0;
	char get[2048]={0};
	char* sen=(char*)malloc(sizeof(char)*1000000);
	if(sen==NULL)
		printf("memory wrong\n");
	if(false==server.bondhost())
	{
        printf("bound wrong\n");
        exit(0);
    }
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
