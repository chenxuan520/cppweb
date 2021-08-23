#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<time.h>
#include<winsock2.h>
#include<urlmon.h>
#include<wininet.h>//-lurlmon -lwininet -lwsock32
class DownloadFile{
private:
	char url[256];
	char urlTop[100];
	char urlEnd[256];
	char fileName[100];
	time_t tbegin;
	time_t tend;
public:
	DownloadFile()
	{
		memset(url,0,256);
		memset(fileName,0,100);
		memset(urlTop,0,100);
		memset(urlEnd,0,256);
		tbegin=0;
		tend=0;
	}
	bool startDownload(const char* httpAdd,int* len,int* rate,const char* urlDownload,const char* toFileName=NULL)
	{
		char* ptemp=NULL;
		strcpy(url,urlDownload);
		if(strstr(urlDownload,"http://")==NULL&&strstr(urlDownload,"https://")==NULL&&httpAdd!=NULL)
			sprintf(url,"http://%s%s",httpAdd,urlDownload);
		if(toFileName!=NULL)
			strcpy(fileName,toFileName);
		else
		{
			if((ptemp=strrchr(urlDownload,'/'))==NULL)
				return false;	
			else
				sscanf(ptemp+1,"%s",fileName);
		}
		DeleteUrlCacheEntry(url);
		tbegin=time(NULL);
		long status=URLDownloadToFile(0,url,fileName,0,NULL);
		if(status==0)
		{
			tend=time(NULL);
			FILE* fp=fopen(fileName,"rb");
			if(fp==NULL)
			{
				*len=0;
				*rate=0;
				return false;
			}
			fseek(fp,0,SEEK_END);
			*len=(int)(ftell(fp)/1024);
			fclose(fp);
			*rate=(int)((*len)/(tend-tbegin));
			return true;	
		}
		*len=0;
		*rate=0;
		return false;	
	}	
	void dealUrl(const char* url,char* urlTop,char* urlEnd)
	{
		char* ptemp=NULL;
		int len=0;
		if((ptemp=strstr(url,"http://"))==NULL)
		{
			if(strstr(url,"https://")!=NULL)
			{
				sscanf(url+8,"%[^/]",urlTop);
				len=strlen(urlTop);
				sscanf(url+len+7,"%s",urlEnd);
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
	void createAsk(const char* domain,char* pask)
	{
		dealUrl(domain,urlTop,urlEnd);
		sprintf(pask,"GET %s HTTP/1.1\r\n"
			"Host:%s\r\n"
			"Connection: Close\r\n\r\n",urlEnd,urlTop);
	}
	void getTopAndEnd(char* top,char* end)
	{
		strcpy(top,urlTop);
		strcpy(end,urlEnd);
	}
	char* getUrl(const char* findFromFile,const char* findRec,char* url,const char* pbegin,const char* pend)
	{
		int i=0;
		char* begin=NULL,*end=NULL,*ptemp=NULL;
		const char* findFrom=findFromFile;
		while(1)
		{
			if((ptemp=strstr(findFrom,findRec))==NULL)
				return NULL;
			if(*(ptemp-1)!='.')
			{
				findFrom+=strlen(findRec);
				continue;
			}
			if((*(ptemp+strlen(findRec))!='\"')&&(*(ptemp+strlen(findRec))!=' '))
			{
				findFrom+=strlen(findRec);
				continue;
			}
			break;				
		}
		begin=ptemp;
		end=ptemp;
		while(*(--begin)!='\"'&&(*begin)!='\''&&(*begin)!=';'&&(*begin)!='='&&begin>pbegin);
		begin++;
		while(*(++end)!='\"'&&(*end)!='\''&&(*end)!=';'&&(*end)!='='&&(end<pend-5));
		for(char* pi=begin;pi<end;pi++)
			url[i++]=*pi;
		url[i]=0;
		return ptemp+strlen(findRec);	
	}
	void reptile(const char* from,const char* kind,int* plen,int* prate)
	{
		const char* ptemp=from;
		while((ptemp=this->getUrl(ptemp,kind,url,from,from+strlen(from)))!=NULL)
			this->startDownload(urlTop,plen,prate,url);
	}
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
	int receive(void* prec,int len)
	{
		return recv(sock,(char*)prec,len,0);
	}
	bool sendHost(const void* ps,int len)
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
};
bool getDnsIp(const char* name,char* ip)
{
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
		exit(0);
	hostent* phost=gethostbyname(name);
	if(phost==NULL)
		return false;
	in_addr addr;
	char* p=phost->h_addr_list[0];
	memcpy(&addr.S_un.S_addr,p,phost->h_length);
	strcpy(ip,inet_ntoa(addr));
//	memcpy(ip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
	return true;
}
enum Choice{
	REPTILE=0,DOWN=1,FROMFILE=2
};
Choice chooseMode()
{
	int model=0;
	system("title Reptile");
	printf("plaese choose model you wang to do reptile(0) or down(1) or fromfile(2):");
	while(scanf("%d",&model)!=1||(model!=0&&model!=1&&model!=2))
	{
		printf("input wrong,input again:");
		fflush(stdin);
	}
	if(model==0)
		return REPTILE;
	else if(model==1)
		return DOWN;
	else
		return FROMFILE;
	return REPTILE;
}
void downloadFile()
{
	char url[256]={0};
	int len=0,rate=0;
	DownloadFile downer;
	while(1)
	{
		fflush(stdin);
		printf("please input the url you want to download:");
		scanf("%s",url);
		if(false==downer.startDownload(NULL,&len,&rate,url))
		{
			printf("download %s wrong\n",url);
			perror("download");
		}
		else
			printf("file:%s ok,%d kb,%d kb/s\n",url,len,rate);
	}
}
void reptile()
{
	int temp=0,len=0,rate=0;
	char url[256]={0},kind[30]={0},urlTop[100]={0},urlEnd[256]={0},ask[256]={0},*pmemory=NULL,*ptemp=NULL;
	DownloadFile downer;
	printf("please input the ip or domain name:");
	scanf("%s",url);
	printf("please input the kind of you want to get:");
	scanf("%s",kind);
	downer.createAsk(url,ask);
	printf("%s",ask);
	downer.getTopAndEnd(urlTop,urlEnd);
	if(false==getDnsIp(urlTop,url))
		perror("ip");
	printf("ip:%s\n",url);
	ClientTcpIp client(url,80);
	pmemory=(char*)malloc(sizeof(char)*100000);
	if(pmemory==NULL)
	{
		fflush(stdin);
		perror("connect");
		getchar();		
	}
	memset(pmemory,0,sizeof(char)*100000);
	if(false==client.tryConnect())
	{
		fflush(stdin);
		perror("connect");
		getchar();
	}
	if(false==client.sendHost(ask,strlen(ask)))
	{
		fflush(stdin);
		perror("connect");
		getchar();
	}	
	if(-1==client.receive(pmemory,sizeof(char)*100000))
	{
		fflush(stdin);
		perror("connect");
		getchar();
	}	
	ptemp=pmemory;
	FILE* fp=fopen("get.txt","w+");
	fprintf(fp,"**\n%s**\n",pmemory);
	fclose(fp);
	while((ptemp=downer.getUrl(ptemp,kind,url,pmemory,pmemory+strlen(pmemory)))!=NULL)
	{
		printf("%s\n",url);
		if(false==downer.startDownload(urlTop,&len,&rate,url))
			perror("catch wrong");
		else 
			printf(" down ok\n",url);
	}
	free(pmemory);
	printf("reptile ok\n");
	fflush(stdin);
	getchar();
	return;
}
void downFile()
{
	int len=0,rate=0;
	char kind[30]={0},url[256]={0},urlTop[100]={0},urlEnd[256]={0};
	DownloadFile downer;
	printf("please input kind you want:");
	scanf("%s",kind);
	printf("please input where you are:");
	scanf("%s",url);
	downer.dealUrl(url,urlTop,urlEnd);
	printf("%s\n",urlTop);
	FILE* fp=fopen("file.txt","rb");
	char* pfile=NULL,*ptemp=NULL;
	if(fp==NULL)
	{
		printf("please create file.txt and put html on it\n");
		fflush(stdin);
		getchar();
		return;
	}
	fseek(fp,0,SEEK_END);
	len=ftell(fp);
	fseek(fp,0,SEEK_SET);
	pfile=(char*)malloc(sizeof(char)*(len+10));
	if(pfile==NULL)
	{
		perror("malloc");
		fflush(stdin);
		getchar();
		return;		
	}
	memset(pfile,0,sizeof(char)*(len+10));
	for(int i=0;i<len;i++)
		pfile[i]=fgetc(fp);
	ptemp=pfile;
	while((ptemp=downer.getUrl(ptemp,kind,url,pfile,pfile+strlen(pfile)))!=NULL)
	{
		printf("%s\n",url);
		if(false==downer.startDownload(urlTop,&len,&rate,url))
			perror("catch wrong");
		else 
			printf(" down ok\n",url);
	}
	printf(" down finish\n",url);
	free(pfile);
	fflush(stdin);
	getchar();
}
int main()
{
	switch(chooseMode())
	{
		case REPTILE:
			reptile();
			break;
		case DOWN:
			downloadFile();
			break;
		case FROMFILE:
			downFile();
	}
	return 0;
}
