#include<iostream>
#include<cstdlib>
#include<stdio.h>
#include<time.h>
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
	date:2021/11/3
	funtion:deal json data
*********************************/
class Json{
private:
	char* buffer;
	char word[30];
	const char* text;
	const char* obj;
	unsigned int nowLen;
	unsigned int maxLen;
public:
	enum TypeJson{
		INT=0,FLOAT=1,ARRAY=2,OBJ=3,STRING=4,STRUCT=5,
	};
	struct Object{
		TypeJson type;
		TypeJson arrTyp;
		const char* key;
		int valInt;
		float valFlo;
		unsigned int floOut;
		unsigned int arrLen;
		const char* valStr;
		void** array;
		Object* pobj;
	public:
		Object()
		{
			type=INT;
			arrTyp=INT;
			key=NULL;
			valFlo=0;
			valInt=0;
			valStr=NULL;
			array=NULL;
			pobj=NULL;
		}
	};
public:
	Json()
	{
		buffer=NULL;
		text=NULL;
		obj=NULL;
		nowLen=0;
		maxLen=0;
		memset(this->word,0,sizeof(char)*30);
	}
	Json(const char* jsonText)
	{
		text=jsonText;
		buffer=NULL;
		nowLen=0;
		maxLen=0;
		memset(this->word,0,sizeof(char)*30);		
	}
	~Json()
	{
		if(this->buffer!=NULL)
			free(buffer);
	}
	bool init(unsigned int bufferLen)
	{
		if(bufferLen<=10)
			return false;
		buffer=(char*)malloc(sizeof(char)*bufferLen);
		memset(buffer,0,sizeof(char)*bufferLen);
		if(buffer==NULL)
			return false;
		this->maxLen=bufferLen;
		strcat(this->buffer,"{");
		this->nowLen+=2;
		return true;
	}
	void addOBject(const Object& obj)
	{
		switch(obj.type)
		{
			case INT:
				this->addKeyValInt(obj.key,obj.valInt);
				break;
			case FLOAT:
				this->addKeyValFloat(obj.key,obj.valFlo,obj.floOut);
				break;
			case STRING:
				this->addKeyValue(obj.key,obj.valStr);
				break;
			case ARRAY:
				this->addArray(obj.arrTyp,obj.key,obj.array,obj.arrLen,obj.floOut);
				break;
			case OBJ:
			case STRUCT:
				strcat(this->buffer,"\"");
				strcat(this->buffer,obj.key);
				strcat(this->buffer,"\":{");
				for(unsigned int i=0;i<obj.arrLen;i++)
					this->addOBject(obj.pobj[0]);
				strcat(this->buffer,"}");
				break;
		}
	}
	bool addKeyValue(const char* key,const char* value)
	{
		char temp[200]={0};
		if(nowLen+strlen(key)+strlen(value)>maxLen)
			return false;
		if(strlen(key)+strlen(value)>=180)
			return false;
		if(buffer[strlen(buffer)-1]=='}')
			buffer[strlen(buffer)-1]=',';
		int len=sprintf(temp,"\"%s\":\"%s\"}",key,value);
		strcat(this->buffer,temp);
		nowLen+=len;
		return true;
	}
	bool addKeyValInt(const char* key,int value)
	{
		char temp[50]={0};
		if(nowLen+50>maxLen)
			return false;
		if(strlen(key)>=45)
			return false;	
		if(buffer[strlen(buffer)-1]=='}')
			buffer[strlen(buffer)-1]=',';
		int len=sprintf(temp,"\"%s\":%d}",key,value);
		strcat(this->buffer,temp);
		nowLen+=len;
		return true;	
	}
	bool addKeyObj(const char* key,const char* value)
	{
		char temp[1000]={0};
		if(nowLen+strlen(key)+strlen(value)>maxLen)
			return false;
		if(strlen(key)+strlen(value)>=980)
			return false;
		if(buffer[strlen(buffer)-1]=='}')
			buffer[strlen(buffer)-1]=',';
		int len=sprintf(temp,"\"%s\":%s}",key,value);
		strcat(this->buffer,temp);
		nowLen+=len;
		return true;		
	}
	bool addArray(TypeJson type,const char* key,void** array,unsigned int arrLen,unsigned int floatNum=1)
	{
		char temp[1000]={0};
		int len=0;
		if(array==NULL||arrLen==0)
			return false;
		if(buffer[strlen(buffer)-1]=='}')
			buffer[strlen(buffer)-1]=',';
		sprintf(temp,"\"%s\":[",key);
		strcat(buffer,temp);
		int* arr=(int*)array;
		float* arrF=(float*)array;
		Object* pobj=(Object*)array;
		switch(type)
		{
			case OBJ:
				for(unsigned int i=0;i<arrLen;i++)
				{
					strcat(buffer,"{");
					switch(pobj[i].type)
					{
						case OBJ:
							this->addOBject(pobj[i]);
							break;
						case INT:
							this->addKeyValInt(pobj[i].key,pobj[i].valInt);
							break;
						case STRING:
							this->addKeyValue(pobj[i].key,pobj[i].valStr);
							break;
						case FLOAT:
							this->addKeyValFloat(pobj[i].key,pobj[i].valFlo,pobj[i].floOut);
							break;
						case ARRAY:
							this->addArray(pobj[i].arrTyp,pobj[i].key,pobj[i].array,pobj[i].arrLen,pobj[i].floOut);
							break;
						case STRUCT:
							strcat(buffer,pobj[i].valStr);
							break;
					}
					strcat(buffer,",");
				}
				buffer[strlen(buffer)-1]=']';
				strcat(buffer,"}");
				nowLen+=len;
				break;
			case STRING:
				for(unsigned int i=0;i<arrLen;i++)
				{
					len=sprintf(temp,"\"%s\",",(char*)array[i]);
					strcat(buffer,temp);
				}
				buffer[strlen(buffer)-1]=']';
				strcat(buffer,"}");
				nowLen+=len;
				break;				
			case INT:
				for(unsigned int i=0;i<arrLen;i++)
				{
					len=sprintf(temp,"%d,",arr[i]);
					strcat(buffer,temp);
				}
				buffer[strlen(buffer)-1]=']';
				strcat(buffer,"}");
				nowLen+=len;
				break;
			case FLOAT:
				for(unsigned int i=0;i<arrLen;i++)
				{
					len=sprintf(temp,"%.*f,",floatNum,arrF[i]);
					strcat(buffer,temp);
				}
				buffer[strlen(buffer)-1]=']';
				strcat(buffer,"}");
				nowLen+=len;
				break;
			case STRUCT:
				for(unsigned int i=0;i<arrLen;i++)
				{
					strcat(buffer,(char*)array[i]);
					len+=strlen((char*)array[i]);
					strcat(buffer,",");	
				}
				buffer[strlen(buffer)-1]=']';
				strcat(buffer,"}");
				nowLen+=len;
				break;
			default:
				return false;
		}
		return true;
	}
	bool addKeyValFloat(const char* key,float value,int output)
	{
		char temp[70]={0};
		if(nowLen+50>maxLen)
			return false;
		if(strlen(key)>=45)
			return false;
		if(buffer[strlen(buffer)-1]=='}')
			buffer[strlen(buffer)-1]=',';
		int len=sprintf(temp,"\"%s\":%.*f}",key,output,value);
		strcat(this->buffer,temp);
		nowLen+=len;
		return true;		
	}
	void createObject(char* pbuffer,int bufferLen,const Object& obj)
	{
		switch(obj.type)
		{
			case INT:
				this->createObjInt(pbuffer,bufferLen,obj.key,obj.valInt);
				break;
			case FLOAT:
				this->createObjFloat(pbuffer,bufferLen,obj.key,obj.valFlo,obj.floOut);
				break;
			case STRING:
				this->createObjValue(pbuffer,bufferLen,obj.key,obj.valStr);
				break;
			case ARRAY:
				this->createObjArray(pbuffer,bufferLen,obj.arrTyp,obj.key,obj.array,obj.arrLen,obj.floOut);
				break;
			case OBJ:
			case STRUCT:
				strcat(this->buffer,"\"");
				strcat(this->buffer,obj.key);
				strcat(this->buffer,"\":{");
				for(unsigned int i=0;i<obj.arrLen;i++)
					this->addOBject(obj.pobj[0]);
				strcat(this->buffer,"}");
				break;
		}	
	}
	int createObjInt(char* pbuffer,unsigned int bufferLen,const char* key,int value)
	{
		if(pbuffer[strlen(pbuffer)-1]=='}')
			pbuffer[strlen(pbuffer)-1]=',';
		if(strlen(pbuffer)==0)
			strcat(pbuffer,"{");
		if(bufferLen<strlen(pbuffer)+strlen(key))
			return -1;
		char temp[100]={0};
		int len=sprintf(temp,"\"%s\":%d}",key,value);
		strcat(pbuffer,temp);
		return len;
	}
	int createObjFloat(char* pbuffer,unsigned int bufferLen,const char* key,float value,int output=1)
	{
		if(pbuffer[strlen(pbuffer)-1]=='}')
			pbuffer[strlen(pbuffer)-1]=',';
		if(strlen(pbuffer)==0)
			strcat(pbuffer,"{");
		if(bufferLen<strlen(pbuffer)+strlen(key))
			return -1;
		char temp[100]={0};
		int len=sprintf(temp,"\"%s\":%.*f}",key,output,value);
		strcat(pbuffer,temp);
		return len;
	}
	int createObjValue(char* pbuffer,unsigned int bufferLen,const char* key,const char* value)
	{
		char temp[200]={0};
		if(strlen(pbuffer)+strlen(key)+strlen(value)>bufferLen)
			return false;
		if(strlen(key)+strlen(value)>=180)
			return false;
		if(pbuffer[strlen(pbuffer)-1]=='}')
			pbuffer[strlen(pbuffer)-1]=',';
		if(strlen(pbuffer)==0)
			strcat(pbuffer,"{");
		int len=sprintf(temp,"\"%s\":\"%s\"}",key,value);
		strcat(pbuffer,temp);
		return len;
	}
	bool createObjArray(char* pbuffer,unsigned int bufferLen,TypeJson type,const char* key,void** array,unsigned int arrLen,unsigned int floatNum=1)
	{
		char temp[200]={0};
		if(array==NULL||arrLen==0)
			return false;
		if(strlen(pbuffer)+strlen(key)>bufferLen)
			return false;
		if(pbuffer[strlen(pbuffer)-1]=='}')
			pbuffer[strlen(pbuffer)-1]=',';
		if(strlen(pbuffer)==0)
			strcat(pbuffer,"{");
		sprintf(temp,"\"%s\":[",key);
		strcat(pbuffer,temp);
		int* arr=(int*)array;
		float* arrF=(float*)array;
		switch(type)
		{
			case STRING:
				for(unsigned int i=0;i<arrLen;i++)
				{
					sprintf(temp,"\"%s\",",(char*)array[i]);
					strcat(pbuffer,temp);
				}
				pbuffer[strlen(pbuffer)-1]=']';
				strcat(pbuffer,"}");
				break;
			case INT:
				for(unsigned int i=0;i<arrLen;i++)
				{
					sprintf(temp,"%d,",arr[i]);
					strcat(pbuffer,temp);
				}
				pbuffer[strlen(pbuffer)-1]=']';
				strcat(pbuffer,"}");
				break;
			case FLOAT:
				for(unsigned int i=0;i<arrLen;i++)
				{
					sprintf(temp,"%.*f,",floatNum,arrF[i]);
					strcat(pbuffer,temp);
				}
				pbuffer[strlen(pbuffer)-1]=']';
				strcat(pbuffer,"}");
				break;
			default:
				return false;
		}
		return true;
	}
	int createObjObj(char* pbuffer,unsigned int bufferLen,const char* key,const char* value)
	{
		char temp[500]={0};
		if(strlen(pbuffer)+strlen(key)+strlen(value)>bufferLen)
			return false;
		if(strlen(key)+strlen(value)>=490)
			return false;
		if(pbuffer[strlen(pbuffer)-1]=='}')
			pbuffer[strlen(pbuffer)-1]=',';
		if(strlen(pbuffer)==0)
			strcat(pbuffer,"{");
		int len=sprintf(temp,"\"%s\":%s}",key,value);
		strcat(pbuffer,temp);
		nowLen+=len;
		return true;
	}
	inline const char* resultText()
	{
		return buffer;
	}
	bool jsonToFile(const char* fileName)
	{
		FILE* fp=fopen(fileName,"w+");
		if(fp==NULL)
			return false;
		fprintf(fp,"%s",this->buffer);
		fclose(fp);
		return true;
	}
	const char* operator[](const char* key)
	{
		char* temp=NULL;
		if((temp=strstr((char*)this->text,key))==NULL)
			return NULL;
		temp=strchr(temp,'\"');
		if(temp==NULL)
			return NULL;
		temp=strchr(temp+1,'\"');
		if(temp==NULL)
			return NULL;
		temp++;
		if(strchr(temp,'\"')-temp>30)
			return NULL;
		memset(this->word,0,sizeof(char)*30);
		for(unsigned int i=0;*temp!='\"';i++,temp++)
			word[i]=*temp;
		return word;
	}
	float getValueFloat(const char* key,bool& flag)
	{
		float value=0;
		char* temp=strstr((char*)text,key);
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		temp=strchr(temp,'\"');
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		temp=strchr(temp+1,':');
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		if(sscanf(temp+1,"%f",&value)<=0)
		{
			flag=true;
			return -1;
		}
		flag=true;
		return value;
	}
	int getValueInt(const char* key,bool& flag)
	{
		int value=0;
		char* temp=strstr((char*)text,key);
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		temp=strchr(temp,'\"');
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		temp=strchr(temp+1,':');
		if(temp==NULL)
		{
			flag=false;
			return -1;
		}
		if(sscanf(temp+1,"%d",&value)<=0)
		{
			flag=true;
			return -1;
		}
		flag=true;
		return value;
	}
};
/********************************
	author:chenxuan
	date:2021.8.1
	funtion:this file is all class and zhushi
*********************************/
class ServerTcpIp{
public:
	enum Thing{
		OUT=0,IN=1,SAY=2,
	};
protected:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now;
	int fd_count;//sum of clients in fd_set
	int last_count;//last fd_count
	int epfd;//file descriptor to ctrl epoll
	char* hostip;//host IP 
	char* hostname;//host name
	const char* error;//error hapen
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
			error="event wrong";
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
	~ServerTcpIp()//clean server
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
	inline const char* lastError()
	{
		return this->error;
	}
	bool selectModel(void* pget,int len,void* pneed,int (*pfunc)(Thing ,int ,int,void* ,void*,ServerTcpIp& ))
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
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
	bool updateSocket(int* array,int* pcount,int arrayLen)//get epoll array
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow&&i<arrayLen;i++)
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
	bool epollModel(void* pget,int len,void* pneed,int (*pfunc)(Thing,int ,int ,void* ,void*,ServerTcpIp& ))
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
	char* getSelfName(char* hostname,unsigned int bufferLen)
	{
		char name[300]={0};
		gethostname(name,300);
		if(strlen(name)>=bufferLen)
			return NULL;
		memcpy(hostname,name,strlen(name));
		return hostname;
	}
	static bool getDnsIp(const char* name,char* ip,unsigned int ipMaxLen)
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
		UNKNOWN=0,HTML=1,EXE=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP7=7,JSON=8,
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
		while(1)//95 _ 
			if((*ptemp>47&&*ptemp<58)||(*ptemp>96&&*ptemp<123)||(*ptemp>64&&*ptemp<91)||*ptemp==95||*ptemp==37)
				break;
			else
				ptemp++;
		pend=ptemp;
		while(1)//46 . 47 / 45 - 43 + 37 % 63 ?
			if((*pend>90&&*pend<97&&*pend!=95)||(*pend<48&&*pend!=46&&*pend!=47&&*pend!=45&&*pend!=43&&*pend!=37)||*pend>122||*pend==63)
				break;
			else
				pend++;
		for(char* pi=ptemp;pi<pend&&i<maxWordLen;pi++)
			word[i++]=*pi;
		word[i]=0;
		return word;
	}
	void* customizeAddTop(void* buffer,int bufferLen,int statusNum,int contentLen,const char* contentType="application/json",const char* connection="keep-alive")
	{
		const char* statusEng=NULL;
		switch(statusNum)
		{
			case 200:
				statusEng="OK";
				break;
			case 301:
				statusEng="Moved Permanently";
				break;
			case 404:
				statusEng="Not Found";
				break;
		}
		sprintf((char*)buffer,"HTTP/1.1 %d %s\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: %s\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n",statusNum,statusEng,connection,contentType,contentLen);
		return buffer;
	}
	void* customizeAddHead(void* buffer,int bufferLen,const char* key,const char* value)
	{
		strcat((char*)buffer,key);
		strcat((char*)buffer,": ");
		strcat((char*)buffer,value);
		strcat((char*)buffer,"\r\n");
	}
	int customizeAddBody(void* buffer,int bufferLen,const char* body,unsigned int bodyLen)
	{
		int topLen=0;
		strcat((char*)buffer,"\r\n");
		unsigned int i=0;
		topLen=strlen((char*)buffer);
		if(bufferLen<topLen+bodyLen)
			return -1;
		char* temp=(char*)buffer+strlen((char*)buffer);
		for(i=0;i<bodyLen;i++)
			temp[i]=body[i];
		temp[i+1]=0;
		return topLen+bodyLen;
	}
	bool setCookie(void* buffer,int bufferLen,const char* key,const char* value,int liveTime=-1,const char* path=NULL,const char* domain=NULL)
	{
		char temp[1000]={0};
		if(strlen(key)+strlen(value)>1000)
			return false;
		sprintf(temp,"Set-Cookie: %s=%s;max-age= %d;",key,value,liveTime);
		strcat((char*)buffer,temp);
		if(path!=NULL)
		{
			strcat((char*)buffer,"Path=");
			strcat((char*)buffer,path);
			strcat((char*)buffer,";");
		}
		if(domain!=NULL)
		{
			strcat((char*)buffer,"Domain=");
			strcat((char*)buffer,domain);
			strcat((char*)buffer,";");
		}
		strcat((char*)buffer,"\r\n");
		return true;
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
				"Connection: keep-alive\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length:%d\r\n\r\n"
				"404 no found",(int)strlen("404 no found"));
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
			case JSON:
				*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server LCserver/1.1\r\n"
				"Connection: keep-alive\r\n"
				"Content-Type:application/json\r\n"
				"Content-Length:%d\r\n\r\n",fileLen);
				break;
		}
	}
	bool createSendMsg(FileKind kind,char* buffer,const char* pfile,int* plong)
	{
		int temp=0;
		int len=0,noUse=0;
		if(kind==NOFOUND)
		{
			this->createTop(kind,buffer,&temp,len);
			*plong=len+temp+1;
			return true;
		}
		len=this->getFileLen(pfile);
		if(len==0)
			return false;
		this->createTop(kind,buffer,&temp,len);
		this->findFileMsg(pfile,&noUse,buffer+temp);
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
		else if(strstr(ask,".json"))
	    {
	        if(false==this->createSendMsg(JSON,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;
	    }
	    else 
	    {
	        if(false==this->createSendMsg(UNKNOWN,psend,ask,plen))
	            if(false==this->createSendMsg(NOFOUND,psend,ask,plen))
	                return 0;
	            else 
	                return 2;
	        else
	        	return 1;
		}
	    return 1;
	}
	const char* getKeyValue(const void* message,const char* key,char* value,int maxValueLen,bool onlyFromBody=false)
	{
		char* temp=NULL;
		if(onlyFromBody==false)
			temp=strstr((char*)message,key);
		else 
		{
			temp=strstr((char*)message,"\r\n\r\n");
			if(temp==NULL)
				return NULL;
			temp=strstr(temp,key);
		}
		if(temp==NULL)
			return NULL;
		return this->findBackString(temp,strlen(key),value,maxValueLen);
	}
	const char* getKeyLine(const void* message,const char* key,char* line,int maxLineLen,bool onlyFromBody=false)
	{
		int i=0;
		char* ptemp=NULL;
		if(false==onlyFromBody)
			ptemp=strstr((char*)message,key);
		else
		{
			ptemp=strstr((char*)message,"\r\n\r\n");
			if(ptemp==NULL)
				return NULL;
			ptemp=strstr(ptemp,key);
		}
		if(ptemp==NULL)
			return NULL;
		ptemp+=strlen(key);
		while(*ptemp==' ')
			ptemp++;
		while(*(ptemp++)!='\r'&&i<maxLineLen)
			line[i++]=*ptemp;
		line[i-1]=0;
		return line;
	}
	const char* getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen)
	{
		char* temp=strstr((char*)message,askWay);
		if(temp==NULL)
			return NULL;
		sscanf(temp+strlen(askWay)+1,"%s",buffer);
		return buffer;
	}
	const char* getRouteValue(const void* routeMsg,const char* key,char* value,unsigned int valueLen)
	{
		char* temp=strstr((char*)routeMsg,key);
		if(temp==NULL)
			return NULL;
		return this->findBackString(temp,strlen(key),value,valueLen);
	}
	const char* getWildUrl(const void* getText,const char* route,char* buffer,int maxLen)
	{
		char* temp=strstr((char*)getText,route);
		if(temp==NULL)
			return NULL;
		temp+=strlen(route);
		sscanf(temp,"%s",buffer);
		return buffer;
	}
	int getRecFile(const void* message,char* fileName,int nameLen,char* buffer,int bufferLen)
	{
		char tempLen[20]={0},*end=NULL,*top=NULL;
		int result=0;
		if(NULL==this->getKeyLine(message,"boundary",buffer,bufferLen))
			return 0;
		if(NULL==this->getKeyValue(message,"filename",fileName,nameLen))
			return 0;
		if(NULL==this->getKeyValue(message,"Content-Length",tempLen,20))
			return 0;
		if(0>=sscanf(tempLen,"%d",&result))
			return 0;
		if((top=strstr((char*)message,buffer))==NULL)
			return 0;
		if((top=strstr(top+strlen(buffer),buffer))==NULL)
			return 0;
		if((end=strstr(top+strlen(buffer),buffer))==NULL)
			return 0;
		if((top=strstr(top,"\r\n\r\n"))==NULL)
			return 0;
		if(end-top>bufferLen)
			return 0;
		top+=4;
		end-=2;
		unsigned int i=0;
		for(i=0;top!=end;i++,top++)
			buffer[i]=*top;
		buffer[i+1]=0;
		return result;
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
	static const char* urlDecode(char* srcString)
	{
		char ch=0;
		int temp=0;
		char* buffer=(char*)malloc(sizeof(char)*strlen(srcString));
		if(buffer==NULL)
			return NULL;
		memset(buffer,0,sizeof(char)*strlen(srcString));
		for (unsigned int i=0; i<strlen(srcString); i++) 
		{
		    if (int(srcString[i])==37) 
			{
		        sscanf(srcString+i+1, "%x", &temp);
		        ch=(char)temp;
		        buffer[strlen(buffer)]=ch;
		        buffer[strlen(buffer)+1]=0;
		        i=i+2;
		    } 
			else 
		        buffer[strlen(buffer)]=srcString[i];
		}
		strcpy(srcString,buffer);
		free(buffer);
		return srcString;
	}
};
/********************************
	author:chenxuan
	date:2021/11/5
	funtion:my jwt try
*********************************/
class WebToken{
private:
	char* backString;
	char err[30];
public:
	WebToken()
	{
		backString=NULL;
		memset(err,0,sizeof(char)*30);
		sprintf(err,"no error");
	}
	const char* createToken(const char* key,const char* encryption,char* getString,unsigned int stringLen,unsigned int liveSecond)
	{
		int temp=0;
		if(key==NULL||encryption==NULL||getString==NULL||stringLen<sizeof(char)*strlen(encryption)+30)
		{
			sprintf(err,"input wrong");
			return NULL;
		}	
		if(backString!=NULL)
			free(backString);
		backString=(char*)malloc(sizeof(char)*strlen(encryption)+30);
		memset(backString,0,sizeof(char)*strlen(encryption)+30);
		if(backString==NULL)
		{
			sprintf(err,"malloc wrong");
			return NULL;
		}
		for(unsigned int i=0;i<strlen(key);i++)
			temp+=key[i];
		int end=time(NULL)+liveSecond+temp;
		temp=temp%4;
		for(unsigned int i=0;i<strlen(encryption);i++)
		{
			backString[i]=encryption[i]-temp;
			if(backString[i]==94)
				backString[i]=92;
		}
		char tempString[30]={0},endString[30]={0};
		sprintf(endString,"%d",end);
		for(unsigned int i=0;i<strlen(endString);i++)
			endString[i]+=50;
		sprintf(tempString,"&%s&%c",endString,encryption[0]);
		strcat(backString,tempString);
		strcpy(getString,backString);
		return getString;
	}
	const char* decryptToken(const char* key,const char* token,char* buffer,unsigned int bufferLen)
	{
		char* temp=strchr((char*)token,'&');
		if(temp==NULL||key==NULL||token==NULL||buffer==NULL||bufferLen<strlen(token))
		{
			sprintf(err,"input wrong");
			return NULL;
		}
		if(backString!=NULL)
			free(backString);
		backString=(char*)malloc(sizeof(char)*strlen(token));
		memset(backString,0,sizeof(char)*strlen(token));
		char endString[20]={0};
		if(sscanf(temp+1,"%[^&]",endString)<=0)
		{
			sprintf(err,"get time wrong");
			return NULL;
		}
		int keyTemp=0,end=0;
		for(unsigned int i=0;i<strlen(endString);i++)
			endString[i]-=50;
		sscanf(endString,"%d",&end);
		for(unsigned int i=0;i<strlen(key);i++)
			keyTemp+=key[i];
		end-=keyTemp;
		if(end-time(NULL)<=0)
		{
			sprintf(err,"over time");
			return NULL;
		}
		keyTemp=keyTemp%4;
		unsigned int i=0;
		for(i=0;i+token<temp;i++)
			if(token[i]!=92)
				backString[i]=token[i]+keyTemp;
			else
				backString[i]=97;
		backString[i+1]=0;
		if(backString[0]!=token[strlen(token)-1])
		{
			sprintf(err,"key wrong");
			return NULL;
		}
		strcpy(buffer,backString);
		return buffer;
	}
	inline const char* LastError()
	{
		return err;
	}
};
/********************************
	author:chenxuan
	date:2021/11/7
	funtion:create a email send class
*********************************/
class Email{
private:
	sockaddr_in their_addr;
	bool isDebug;
	char error[30];
    struct Base64Date6
    {
        unsigned int d4 : 6;
        unsigned int d3 : 6;
        unsigned int d2 : 6;
        unsigned int d1 : 6;
    };
public:
	Email(const char* domain,bool debug=false)
	{
		isDebug=debug;
		memset(error,0,sizeof(char)*30);
        memset(&their_addr, 0, sizeof(their_addr));
        their_addr.sin_family = AF_INET;
        their_addr.sin_port = htons(25);    
        hostent* hptr = gethostbyname(domain);          
        memcpy(&their_addr.sin_addr.s_addr, hptr->h_addr_list[0], hptr->h_length);
	}
	bool emailSend(const char* sendEmail,const char* passwd,const char* recEmail,const char* body)
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
	char ConvertToBase64(char uc)
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
    void EncodeBase64(char *dbuf, char *buf128, int len)
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
    void CreateSend(const char* youName,const char* toName,const char* from,const char* to,const char* subject,const char* body,char* buf)
    {
        sprintf(buf,"From: \"%s\"<%s>\r\n"
                "To: \"%s\"<%s>\r\n"
                "Subject:%s\r\n\r\n"
                "%s\n",youName,from,toName,to,subject,body);
    }
    inline const char* LastError()
    {
		return error;
	}
	static const char* getDomainBySelfEmail(const char* email,char* buffer,int bufferLen)
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
};
/********************************
	author:chenxuan
	date:2021/8/10
	funtion:class to deal ddos
*********************************/
class LogSystem{
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
	static bool recordFileError(const char* fileName)
	{
		FILE* fp=fopen("wrong.log","r+");
		if(fp==NULL)
			fp=fopen("wrong.log","w+");
		if(fp==NULL)
			return false;
		fseek(fp,0,SEEK_END);
		fprintf(fp,"open file %s wrong\n",fileName);
		fclose(fp);
		return true;
	}
};
/********************************
	author:chenxuan
	date:2021/11/4
	funtion:the http server for new
*********************************/
class HttpServer:private ServerTcpIp{
public:
	enum RouteType{
		ONEWAY,WILD,
	};
	enum AskType{
		GET,POST,ALL,
	};
	struct RouteFuntion{
		AskType ask;
		RouteType type;
		char route[100];
		void (*pfunc)(DealHttp&,HttpServer&,int num,void* sen,int&);
	};
private:
	RouteFuntion* array;
	void* getText;
	unsigned int max;
	unsigned int now;
	int textLen;
	bool isDebug;
	void (*clientIn)(HttpServer&,int num,void* ip,int port);
	void (*clientOut)(HttpServer&,int num,void* ip,int port);
public:
	HttpServer(unsigned port,bool debug=false):ServerTcpIp(port)
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
		textLen=0;
		clientIn=NULL;
		clientOut=NULL;
	}
	~HttpServer()
	{
		if(array!=NULL)
			free(array);
	}
	bool routeHandle(AskType ask,RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
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
	bool loadStatic(const char* route,const char* staticPath)
	{
		char temp[100]={0};
		sprintf(temp,"%s %s",route,staticPath);
		this->get(WILD,temp,loadFile);
	}
	bool get(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
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
	bool post(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
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
	bool all(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
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
	bool clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
	{
		if(clientIn!=NULL)
			return false;
		clientIn=pfunc;
	}
	bool clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
	{
		if(clientOut!=NULL)
			return false;
		clientOut=pfunc;
	}
	void run(unsigned int memory,unsigned int recBufLenChar,const char* defaultFile)
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
		while(1)
			this->epollHttp(get,recBufLenChar,sen,defaultFile);
		free(sen);
		free(get);
	}
	int httpSend(int num,void* buffer,int sendLen)
	{
		return this->sendSocket(num,buffer,sendLen);
	}
	inline void* recText()
	{
		return this->getText;
	}
	inline int recLen()
	{
		return this->textLen;
	}
	inline const char* lastError()
	{
		return error;
	}
	inline bool disconnect(int soc)
	{
		return this->disconnectSocket(soc);
	}
private:
	int func(int num,void* pget,void* sen,const char* defaultFile,HttpServer& server)
	{
		static DealHttp http;
		AskType type=GET;
		int len=0,flag=2;
		char ask[200]={0};
		if(strstr((char*)pget,"GET")!=NULL)
		{
			http.getAskRoute(pget,"GET",ask,200);
			if(isDebug)
				printf("Get url:%s\n",ask);
			type=GET;
		}
		if(strstr((char*)pget,"POST")!=NULL)
		{
			http.getAskRoute(pget,"POST",ask,200);
			if(isDebug)
				printf("POST url:%s\n",ask);
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
			if(http.analysisHttpAsk(pget)!=NULL)
			{
				strcpy(ask,http.analysisHttpAsk(pget));
				flag=http.autoAnalysisGet((char*)pget,(char*)sen,defaultFile,&len);
			}
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
	void epollHttp(void* pget,int len,void* pneed,const char* defaultFile)
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
		memset(pget,0,sizeof(char)*len);
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
				{
					this->textLen=getNum;
					func(temp.data.fd,pget,pneed,defaultFile,*this);
				}
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
	static void loadFile(DealHttp&,HttpServer&,int,void*,int&)
	{
		
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
	static void createDetachPthread(void* arg,void* (*pfunc)(void*))//create a ddetach thread
	{
		pthread_t thread=0;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_create(&thread,&attr,pfunc,arg);
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
	inline void mutexLock()
	{
		pthread_mutex_lock(&mutex);
	}
	inline void mutexUnlock()
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
	bool updateSocket(int* p,int* pcount,int arrayLen)//get epoll array
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow&&arrayLen;i++)
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
    	ServerTcpIpThreadPool::ArgvSer argv={*this,0,pneed};
    	ThreadPool::Task task={pfunc,&argv};
    	while(1)
    	{
			sockaddr_in newaddr={0};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
			if(newClient==-1)
				continue;
			this->addFd(newClient);
			argv.soc=newClient;
    		task.ptask=pfunc;
    		task.arg=&argv;
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
	date:2021/11/5
	funtion:server tcp ip 2.0
*********************************/
class ServerPool:public ServerTcpIp{
private:
	ThreadPool* pool;
	pthread_mutex_t mutex;
public:
	struct ArgvSer{
		ServerPool& server;
		int soc;
		void* pneed;
	};
	struct ArgvSerEpoll{
		ServerPool& server;
		int soc;
		int thing;
		int len;
		void* pneed;
		void* pget;	
	};
public:
	ServerPool(unsigned short port,unsigned int threadNum=0):ServerTcpIp(port)
	{
		if(threadNum>0)
		{	
			pool=new ThreadPool(threadNum);
			if(pool==NULL)
				throw NULL;
		}
		pthread_mutex_init(&mutex,NULL);
	}
	~ServerPool()
	{
		delete pool;
       	pthread_mutex_destroy(&mutex);
	}
	inline void mutexLock()
	{
		pthread_mutex_lock(&mutex);
	}
	inline void mutexUnlock()
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
	void threadModel(void* pneed,void* (*pfunc)(void*))
    {
    	ServerPool::ArgvSer argv={*this,0,pneed};
    	ThreadPool::Task task={pfunc,&argv};
    	while(1)
    	{
			sockaddr_in newaddr={0};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
			if(newClient==-1)
				continue;
			this->addFd(newClient);
			argv.soc=newClient;
    		task.ptask=pfunc;
    		task.arg=&argv;
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
				ServerPool::ArgvSerEpoll argv={*this,*pnum,*pthing,0,pneed,pget};
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
					ServerPool::ArgvSerEpoll argv={*this,*pnum,*pthing,getNum,pneed,pget};
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
	bool getFileMsg(const char* fileName,char* buffer,unsigned int bufferLen)
	{
		int i=0,len=0;
		len=this->getFileLen(fileName);
		FILE* fp=fopen(fileName,"rb");
		if(fp==NULL)
			return false;
		for(i=0;i<len&&i<bufferLen;i++)
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
		if(false==this->getFileMsg(fileName,pbuffer,sizeof(char)*(len+10)))
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
	static bool writeToFile(const char* fileName,const char* buffer,unsigned int writeLen)
	{
		FILE* fp=fopen(fileName,"wb+");
		if(fp==NULL)
			return false;
		for(unsigned int i=0;i<writeLen;i++)
			fputc(buffer[i],fp);
		fclose(fp);
		return true;
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
int funcTwo(ServerPool::Thing thing,int num,int,void* pget,void* sen,ServerTcpIp& server)//main deal func
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
	if(false==LogSystem::dealAttack(thing,num,200))
	{
		LogSystem::attackLog(port,server.getPeerIp(num,&port),"./rec/attackLog.txt");
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
		printf("url:%s\n",http.getAskRoute(pget,"GET",ask,200));
		if(strstr(http.getAskRoute(pget,"GET",ask,200),"/user")!=NULL)
		{
			Json json;
			char data[300]={0};
			json.init(200);
			json.addKeyValue("hahah","lplplp");
			json.addKeyValInt("dad",23);
			printf("json:%s\n",json.resultText());
			json.jsonToFile("./temp");
			FileGet file;
			if(false==file.getFileMsg("./temp",data,300))
				perror("file");
			if(false==http.createSendMsg(DealHttp::JSON,(char*)sen,"./temp",&len))
				perror("create");
			printf("sen %s\n",(char*)sen);
			server.sendSocket(num,sen,len);
			return 0;
		}
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
int funcThree(ServerTcpIp::Thing thing,int num,int,void* pget,void* sen,ServerTcpIp& server)//main deal func
{
	char ask[200]={0},*pask=NULL;
	DealHttp http;
	int len=0;
	int port=0;
	int flag=0;
	if(sen==NULL)
		return -1;
	memset(sen,0,sizeof(char)*10000000);
	if(false==LogSystem::dealAttack(thing,num,200))
	{
		LogSystem::attackLog(port,server.getPeerIp(num,&port),"./rec/attackLog.txt");
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
		printf("url:%s\n",http.getAskRoute(pget,"GET",ask,200));
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
		server.selectModel(get,2048,sen,funcThree);
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
		if(false==server.epollModel(get,2048,sen,funcTwo))
			break;
	free(sen);
}
/********************************
	author:chenxuan
	date:2021/11/5
	funtion:tryfunction
*********************************/
void func(DealHttp& http,HttpServer& server,int num,void* sen,int& len)
{
	char buffer[100]={0},name[30]={0};
	Json json;
	json.init(300);
	http.getKeyValue(server.recText(),"email",buffer,100);
	json.addKeyValue("email",buffer);
	http.getKeyValue(server.recText(),"password",buffer,100);
	json.addKeyValInt("wuwu",90);
	json.addKeyValue("passwd",buffer);
	printf("%s\n",(char*)server.recText());
	json.jsonToFile("temp");
	http.createSendMsg(DealHttp::JSON,(char*)sen,"temp",&len);
}
void funHa(DealHttp& http,HttpServer& server,int num,void* sen,int& len)
{
	char buffer[100]={0},name[30]={0};
	Json json;
	json.init(300);
	json.addKeyValue("key","op");
	json.jsonToFile("temp");
	printf("buffer:%s\n",buffer);
	http.createSendMsg(DealHttp::JSON,(char*)sen,"temp",&len);
}
/********************************
	author:chenxuan
	date:2021/9/9
	funtion:thank you for watching
*********************************/
int main(int argc, char** argv) 
{
//	serverHttp();
	HttpServer server(5201,true);
	server.all(HttpServer::ONEWAY,"/root",func);
	server.routeHandle(HttpServer::ALL,HttpServer::ONEWAY,"/key/",funHa);
	server.run(1 ,4000,"./index.html");
	return 0;
}
