#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<time.h>
#include<winsock2.h>
//#include<openssl/ssl.h>
//#include<openssl/err.h>
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
	bool updateSocket(SOCKET* array,int* pcount)
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow;i++)
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
	static bool getDnsIp(const char* name,char* ip)
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
};//D:/Dev-Cpp/MinGW64/x86_64-w64-mingw32/lib/libws2_32.a
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
int funcTwo(int thing,int num,int,void* pget,void* sen,ServerTcpIp& server)
{
	DealHttp http;
	int len=0;
	if(sen==NULL)
		return -1;
	memset(sen,0,sizeof(char)*10000000);
	if(thing==0)
	{
		printf("%d out %s\n",num,(char*)pget);

	}
	if(thing==1)
	{
		printf("%d in %s\n",num,(char*)pget);
	}
	if(thing==2)
	{
		if(false==http.cutLineAsk((char*)pget,"GET"))
			return 0;
		printf("ask:%s",(char*)pget);
		printf("http:%s\n",http.analysisHttpAsk(pget));
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
void serverHttp()
{
	unsigned int port=80;
	chooseModel(&port);
	ServerTcpIp server(port);
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
