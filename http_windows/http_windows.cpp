#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<time.h>
#include<winsock2.h>
class WSAinit{
public:
	WSAinit()
	{
		WSADATA wsa;//web server api data加载库初始化 
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
class ServerTcpIp{
private:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now; 
	int max;//the most clients;
	char* hostip;//host IP 
	char* hostname;//host name
	WSADATA wsa;//apply verson of windows;
	SOCKET sock;//file descriptor of host;
	SOCKET* psockClients;//client[];
	SOCKET sockC;//file descriptor to sign every client;
	SOCKADDR_IN addr;//IPv4 of host;
	SOCKADDR_IN client;//IPv4 of client;
	fd_set  fdClients;//file descriptor文件描述符
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
	ServerTcpIp(unsigned short port=5200,int wait=5,int maxClient=0)
	{
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
			exit(0);
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
		if(maxClient<=0||maxClient>100)
			psockClients=NULL;
		else 
		{
			psockClients=(SOCKET*)malloc(sizeof(SOCKET)*maxClient);
			if(psockClients==NULL)
				exit(0);
			max=maxClient;
		}
	}
	~ServerTcpIp()//clean server
	{
		if(psockClients!=NULL)
			free(psockClients);
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
	bool acceptClient()//wait until success
	{
		sockC=accept(sock,(sockaddr*)&client,&sizeAddr);
		return true;
	}
	bool acceptClientSTwo(int i)//model two
	{
		if(i<max)
		{
			psockClients[i]=accept(sock,(sockaddr*)&client,&sizeAddr);
			numClient++;
		}	
		else
			return false;
		return true;
	}
	bool receiveOne(void* pget,int len)//model one
	{
		if(recv(sockC,(char*)pget,len,0)==SOCKET_ERROR)
			return false;
		return true;
	}
	bool receiveSTwo(void* prec,int i,int len)//model two
	{
		if(recv(psockClients[i],(char*)prec,len,0)==SOCKET_ERROR)
		{
			psockClients[i]=0;
			return false;
		}
		return true;
	}
	bool sendClient(const void* ps,int len)//model one
	{
		if(send(sockC,(char*)ps,len,0)==SOCKET_ERROR)
			return false;
		return true;
	}
	bool sendClientSTwo(const void* ps,int i,int len)//model two
	{
		if(send(psockClients[i],(char*)ps,len,0)==SOCKET_ERROR)
			return false;
		return true;
	}
	bool sendClientsEveryoneTwo(const void* ps,int len)//model two
	{
		for(int i=0;i<numClient;i++)
		{ 
			if(psockClients[i]==0)
				continue;
			send(psockClients[i],(char*)ps,len,0);
		}
		return true;
	}
	bool selectModel(int* pthing,int* pnum,void* pget,void* pneed,int len,int (*pfunc)(int ,int ,void* ,void*,ServerTcpIp& ))
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
							for(int j=0;j<(int)fdClients.fd_count;j++)
							{
								if(newClient==fdClients.fd_array[j])
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
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						*pnum=i;
						if(sRec>0)
							*pthing=2;
						if(sRec<=0)
						{
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);
							*(char*)pget=NULL;
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
	bool selectSend(const void* ps,int num,int len)
	{
		SOCKET clientSent=fdClients.fd_array[num];
		if(send(fdClients.fd_array[num],(char*)ps,len,0)==SOCKET_ERROR)
			return false;
		return true;
	}
	bool selectSendEveryone(void* ps,int len)
	{
		for(int i=0;i<fdClients.fd_count;i++)
		{ 
			SOCKET clientSent=fdClients.fd_array[i];
			if(clientSent!=0)
				send(fdClients.fd_array[i],(char*)ps,len,0);
		}
		return true;
	}
	bool updateSocketSelect(SOCKET* p,int* pcount)
	{
		if(fdClients.fd_count!=0)
			*pcount=fdClients.fd_count;
		else
			return false;
		for(int i=0;i<fdClients.fd_count;i++)
			p[i]=fdClients.fd_array[i];
		return true;
	}
	bool sendSocketSelect(SOCKET toClient,const void* ps,int len)
	{
		for(int i=0;i<fdClients.fd_count;i++)
		{
			if(toClient==fdClients.fd_array[i])
			{
				send(fdClients.fd_array[i],(char*)ps,len,0);
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
    bool sendSocketAll(int socCli,const void* ps,int len)
	{
		if(send(socCli,(char*)ps,len,0)==-1)
			return false;
		return true;
	}
    bool disconnectSocket(SOCKET clisock)
    {
        for(int i=0;i<fdClients.fd_count;i++)
        {
            if(fdClients.fd_array[i]==clisock)
            {
                closesocket(clisock);
                FD_CLR(fdClients.fd_array[i],&fdClients);
                return true;
            }
        }
        return false;
    }
	int getSocketArray(int* socArray)
	{
		for(int i=0;i<fdNumNow;i++)
			socArray[i]=pfdn[i];
		return fdNumNow;
	}
	bool findSocketArray(int cliSoc)
	{
		for(int i=0;i<fdNumNow;i++)
			if(pfdn[i]==cliSoc)
				return true;
		return false;
	}
	char* getHustIp()
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
public:
	ClientTcpIp(const char* ps,int port=5200)
	{
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
			exit(0);
		memset(ip,0,20);
		strcpy(ip,ps);
		sock=socket(AF_INET,SOCK_STREAM,0);
		addrC.sin_addr.S_un.S_addr=inet_addr(ip);
		addrC.sin_family=AF_INET;
		addrC.sin_port=htons(port);
		hostip=(char*)malloc(sizeof(char)*20);
		memset(hostip,0,sizeof(char)*20);
		hostname=(char*)malloc(sizeof(char)*30);
		memset(hostname,0,sizeof(char)*30);
	}
	~ClientTcpIp()
	{
		free(hostip);
		free(hostname);
		closesocket(sock);
		WSACleanup();
	}
	bool tryConnect()
	{
		if(connect(sock,(SOCKADDR*)&addrC,sizeof(sockaddr))==SOCKET_ERROR)
			return false;
		return true;
	}
	bool receiveMystl(void* prec,int len)
	{
		if(recv(sock,(char*)prec,len,0)==SOCKET_ERROR)
			return false;
		return true;
	}
	bool sendHostMystl(const void* ps,int len)
	{
		if(send(sock,(char*)ps,len,0)==SOCKET_ERROR)
			return false;
		return true;
	}
	bool updateSocket()
	{
		closesocket(sock);
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(sock==-1)
			return false;
		return true;
	}
	char* getHustIp()
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
};//D:/Dev-Cpp/MinGW64/x86_64-w64-mingw32/lib/libws2_32.a
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
		else if(strstr(ask,".css"))
		{
			if(false==this->createSendMsg(5,psend,ask,plen))
				if(false==this->createSendMsg(4,psend,ask,plen))
					return false;
		}
		else if(strstr(ask,".js"))
		{
			if(false==this->createSendMsg(6,psend,ask,plen))
				if(false==this->createSendMsg(4,psend,ask,plen))
					return false;
		}
		else if(strstr(ask,".7z"))
		{
			if(false==this->createSendMsg(7,psend,ask,plen))
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
	char hao[20]={0},mi[20]={0},min[20]={0};
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
int funcTwo(int thing,int num,void* pget,void* sen,ServerTcpIp& server)
{
	char ask[20]={0},*pask=NULL;
	CliMsg cli;
	DealHttp http;
	int len=0;
	if(sen==NULL)
		return -1;
	memset(sen,0,sizeof(char)*10000000);
	if(thing==0)
	{
		

	}
	if(thing==1)
	{
		
	}
	if(thing==2)
	{
		if(false==http.cutLineAsk((char*)pget,"GET"))
			return 0;
		printf("ask:%s\n",(char*)pget);
		printf("http:%s\n",http.analysisHttpAsk(pget));
		if(false==http.autoAnalysisGet((char*)pget,(char*)sen,indexName,&len))
			printf("some thing wrong %s\n",sen);
		else
			printf("create auto success\n");
		if(false==server.selectSend(sen,num,len))
			printf("send wrong\n");
		else
			printf("send success\n");
	}
	return 0;
}
void serverHttp()
{
	unsigned int port=80;
	bool is_back=false;
	chooseModel(&port);
	ServerTcpIp server(80);
	int thing=0,num=0;
	char get[2048]={0};
	char* sen=(char*)malloc(sizeof(char)*10000000);
	if(sen==NULL)
		printf("memory wrong\n");
	system("title web server");
	if(false==server.bondhost())
		exit(0);
	if(false==server.setlisten())
		exit(0);
	printf("server ip is:%s\nthe server is ok\n",server.getHustIp());
	while(1)
		if(false==server.selectModel(&thing,&num,get,sen,2048,funcTwo))
			break;
	free(sen);
}
int main(int argc, char** argv) 
{
	serverHttp();
	return 0;
}
