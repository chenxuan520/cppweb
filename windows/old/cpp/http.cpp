#include"../hpp/http.h"
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<stdio.h>
using namespace std;
namespace cppweb{
DealHttp::DealHttp()
{
	for(int i=0;i<256;i++)
		ask[i]=0;
	pfind=NULL;
	error=NULL;
}
bool DealHttp::cutLineAsk(char* pask,const char* pcutIn)
{
	if(pask==NULL||pcutIn==NULL)
	{
		error="wrong NULL";
		return false;
	}
	char* ptemp=strstr(pask,pcutIn);
	if(ptemp==NULL)
		return false;
	while(*(ptemp++)!='\n');
	*ptemp=0;
	return true;
}
const char* DealHttp::analysisHttpAsk(void* message,const char* pneed)
{
	if(message==NULL)
	{
		error="wrong NULL";
		return NULL;
	}
	pfind=strstr((char*)message,pneed);
	if(pfind==NULL)
		return NULL;
	return this->findBackString(pfind,strlen(pneed),ask,256);
}
char* DealHttp::findBackString(char* local,int len,char* word,int maxWordLen)
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
void* DealHttp::customizeAddTop(void* buffer,unsigned int bufferLen,int statusNum,unsigned int contentLen,const char* contentType,const char* connection,const char* staEng)
{
	const char* statusEng=NULL;
	if(bufferLen<100)
		return NULL;
	switch(statusNum)
	{
		case 200:
			statusEng="OK";
			break;
		case 204:
			statusEng="No Content";
			break;
		case 301:
			statusEng="Moved Permanently";
			break;
		case 400:
			statusEng="Bad Request";
			break;
		case 403:
			statusEng="Forbidden";
			break;
		case 404:
			statusEng="Not Found";
			break;
		case 501:
			statusEng="Not Implemented";
			break;
		default:
			statusEng=staEng;
			break;
	}
	sprintf((char*)buffer,"HTTP/1.1 %d %s\r\n"
		"Server LCserver/1.1\r\n"
		"Connection: %s\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %d\r\n",statusNum,statusEng,connection,contentType,contentLen);
	return buffer;
}
void* DealHttp::customizeAddHead(void* buffer,unsigned int bufferLen,const char* key,const char* value)
{
	if(strlen((char*)buffer)+strlen(key)+strlen(value)+4>=bufferLen)
		return NULL;
	strcat((char*)buffer,key);
	strcat((char*)buffer,": ");
	strcat((char*)buffer,value);
	strcat((char*)buffer,"\r\n");
	return buffer;
}
int DealHttp::customizeAddBody(void* buffer,unsigned int bufferLen,const char* body,unsigned int bodyLen)
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
bool DealHttp::setCookie(void* buffer,unsigned int bufferLen,const char* key,const char* value,int liveTime,const char* path,const char* domain)
{
	char temp[1000]={0};
	if(strlen(key)+strlen(value)>1000)
		return false;
	sprintf(temp,"Set-Cookie: %s=%s;max-age= %d;",key,value,liveTime);
	if(strlen((char*)buffer)+strlen(temp)>=bufferLen)
		return false;
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
const char* DealHttp::getCookie(void* recText,const char* key,char* value,unsigned int valueLen)
{
	if(recText==NULL||key==NULL||value==NULL||valueLen==0)
		return NULL;
	char* temp=strstr((char*)recText,"\r\n\r\n"),*cookie=NULL;
	if(temp==NULL)
		return NULL;
	*temp=0;
	cookie=strstr((char*)recText,"Cookie");
	if(cookie==NULL)
		return NULL;
	cookie=strstr(cookie,key);
	if(cookie==NULL)
		return NULL;
	this->findBackString(cookie,strlen(key),value,valueLen);
	*temp='\r';
	return value;
}
void DealHttp::createTop(FileKind kind,char* ptop,unsigned int bufLen,int* topLen,unsigned int fileLen)//1:http 2:down 3:pic
{
	if(bufLen<100)
	{
		this->error="buffer too short";
		return;
	}
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
		case ZIP:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:application/zip\r\n"
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
bool DealHttp::createSendMsg(FileKind kind,char* buffer,unsigned int bufferLen,const char* pfile,int* plong)
{
	int temp=0;
	int len=0,noUse=0;
	if(kind==NOFOUND)
	{
		this->createTop(kind,buffer,bufferLen,&temp,len);
		*plong=len+temp+1;
		return true;
	}
	len=this->getFileLen(pfile);
	if(len==0)
		return false;
	this->createTop(kind,buffer,bufferLen,&temp,len);
	this->findFileMsg(pfile,&noUse,buffer+temp,bufferLen);
	*plong=len+temp+1;
	return true;
}
char* DealHttp::findFileMsg(const char* pname,int* plen,char* buffer,unsigned int bufferLen)
{
	FILE* fp=fopen(pname,"rb+");
	unsigned int flen=0,i=0;
	if(fp==NULL)
		return NULL;
	fseek(fp,0,SEEK_END);
	flen=ftell(fp);
	if(flen>=bufferLen)
	{
		this->error="buffer too short";
		fclose(fp);
		return NULL;
	}
	fseek(fp,0,SEEK_SET);
	for(i=0;i<flen;i++)
		buffer[i]=fgetc(fp);
	buffer[i]=0;
	*plen=flen;
	fclose(fp);
	return buffer;
}
int DealHttp::createDatagram(const Datagram& gram,void* buffer,unsigned bufferLen)
{
	if(gram.fileLen>bufferLen||bufferLen==0)
		return -1;
	const char* statusEng=NULL;
	char temp[200]={0};
	if(bufferLen<100||bufferLen<gram.fileLen+100)
	{
		error="len too short";
		return -1;
	}
	switch(gram.statusCode)
	{
	case STATUSOK:
		statusEng="OK";
		break;
	case STATUSNOCON:
		statusEng="No Content";
		break;
	case STATUSMOVED:
		statusEng="Moved Permanently";
		break;
	case STATUSBADREQUEST:
		statusEng="Bad Request";
		break;
	case STATUSFORBIDDEN:
		statusEng="Forbidden";
		break;
	case STATUSNOFOUND:
		statusEng="Not Found";
		break;
	case STATUSNOIMPLEMENT:
		statusEng="Not Implemented";
		break;
	default:
		error="status code UNKNOWN";
		return -1;
	}
	sprintf((char*)buffer,"HTTP/1.1 %d %s\r\n"
		"Server LCserver/1.1\r\n"
		"Connection: keep-alive\r\n",
		gram.statusCode,statusEng);
	if(gram.fileLen==0)
	{
		sprintf((char*)buffer,"\r\n");
		return strlen((char*)buffer);
	}
	switch(gram.typeFile)
	{
	case UNKNOWN:
	case NOFOUND:
		strcat((char*)buffer,"\r\n");
		return strlen((char*)buffer);
	case HTML:
		sprintf(temp,"Content-Type:%s\r\n","text/html");
		break;
	case EXE:
		sprintf(temp,"Content-Type:%s\r\n","application/octet-stream");
		break;
	case IMAGE:
		sprintf(temp,"Content-Type:%s\r\n","image");
		break;
	case CSS:
		sprintf(temp,"Content-Type:%s\r\n","text/css");
		break;
	case JS:
		sprintf(temp,"Content-Type:%s\r\n","text/javascript");
		break;
	case JSON:
		sprintf(temp,"Content-Type:%s\r\n","application/json");
		break;
	case ZIP:
		sprintf(temp,"Content-Type:%s\r\n","application/zip");
		break;
	}
	strcat((char*)buffer,temp);
	sprintf(temp,"Content-Length:%d\r\n",gram.fileLen);
	strcat((char*)buffer,temp);
	if(gram.head.size()!=0)
		for(auto iter=gram.head.begin();iter!=gram.head.end();iter++)
			customizeAddHead(buffer,bufferLen,iter->first.c_str(),iter->second.c_str());
	if(gram.cookie.size()!=0)
		for(auto iter=gram.cookie.begin();iter!=gram.cookie.end();iter++)
			setCookie(buffer,bufferLen,iter->first.c_str(),iter->second.c_str());
	return customizeAddBody(buffer,bufferLen,(char*)gram.body,gram.fileLen);
}
void DealHttp::getRequestMsg(void* message,Request& request)
{
	char method[30]={0},path[256]={0},version[30]={0};
	sscanf((char*)message,"%30s%256s%30[^\r]",method,path,version);
	request.askPath=path;
	request.method=method;
	request.version=version;
}
int DealHttp::getFileLen(const char* pname)
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
int DealHttp::autoAnalysisGet(const char* message,char* psend,unsigned int bufferLen,const char* pfirstFile,int* plen)
{
	if(NULL==this->analysisHttpAsk((void*)message))
		return 0;
	if(strcmp(ask,"HTTP/1.1")==0||strcmp(ask,"HTTP/1.0")==0)
	{
		if(false==this->createSendMsg(HTML,psend,bufferLen,pfirstFile,plen))
		{
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		}
		else
			return 1;
	}
	else if(strstr(ask,".html"))
	{
		if(false==this->createSendMsg(HTML,psend,bufferLen,ask,plen))
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		else
			return 1;
	}
	else if(strstr(ask,".exe"))
	{
		if(false==this->createSendMsg(EXE,psend,bufferLen,ask,plen))
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		else
			return 1;			
	}
	else if(strstr(ask,".zip"))
	{
		if(false==this->createSendMsg(ZIP,psend,bufferLen,ask,plen))
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		else
			return 1;			
	}
	else if(strstr(ask,".png")||strstr(ask,".PNG")||strstr(ask,".jpg")||strstr(ask,".jpeg"))
	{
		if(false==this->createSendMsg(IMAGE,psend,bufferLen,ask,plen))
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		else
			return 1;					
	}
	else if(strstr(ask,".css"))
	{
		if(false==this->createSendMsg(CSS,psend,bufferLen,ask,plen))
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		else
			return 1;					
	}
	else if(strstr(ask,".js"))
	{
		if(false==this->createSendMsg(JS,psend,bufferLen,ask,plen))
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		else
			return 1;
	}
	else if(strstr(ask,".json"))
	{
		if(false==this->createSendMsg(JSON,psend,bufferLen,ask,plen))
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		else
			return 1;
	}
	else 
	{
		if(false==this->createSendMsg(UNKNOWN,psend,bufferLen,ask,plen))
			if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
				return 0;
			else 
				return 2;
		else
			return 1;
	}
	return 1;
}
const char* DealHttp::getKeyValue(const void* message,const char* key,char* value,unsigned int maxValueLen,bool onlyFromBody)
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
const char* DealHttp::getKeyLine(const void* message,const char* key,char* line,unsigned int maxLineLen,bool onlyFromBody)
{
	unsigned int i=0;
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
const char* DealHttp::getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen)
{
	char* temp=strstr((char*)message,askWay);
	if(temp==NULL)
		return NULL;
	char format[20]={0};
	sprintf(format,"%%%us",bufferLen);
	sscanf(temp+strlen(askWay)+1,format,buffer);
	return buffer;
}
const char* DealHttp::getRouteValue(const void* routeMeg,const char* key,char* value,unsigned int valueLen)
{
	char* temp=strstr((char*)routeMeg,key);
	if(temp==NULL)
		return NULL;
	return this->findBackString(temp,strlen(key),value,valueLen);
}
const char* DealHttp::getWildUrl(const void* getText,const char* route,char* buffer,unsigned int maxLen)
	{
	char* temp=strstr((char*)getText,route);
	if(temp==NULL)
		return NULL;
	temp+=strlen(route);
	char format[20]={0};
	sprintf(format,"%%%us",maxLen);
	sscanf(temp,format,buffer);
	return buffer;
}
int DealHttp::getRecFile(const void* message,char* fileName,int nameLen,char* buffer,unsigned int bufferLen)
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
	result=end-top;
	unsigned int i=0;
	for(i=0;top!=end;i++,top++)
		buffer[i]=*top;
	buffer[i+1]=0;
	return result;
}
const char* DealHttp::urlDecode(char* srcString)
{
	char ch=0;
	int temp=0;
	unsigned int srcLen=strlen(srcString);
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
	if(srcLen<strlen(buffer))
	{
		free(buffer);
		return NULL;
	}
	strcpy(srcString,buffer);
	free(buffer);
	return srcString;
}
void DealHttp::dealUrl(const char* url,char* urlTop,char* urlEnd,unsigned int topLen,unsigned int endLen)
{
	const char* ptemp=NULL;
	char format[20]={0};
	int len=0;
	if((ptemp=strstr(url,"http://"))==NULL)
	{
		if(strstr(url,"https://")!=NULL)
		{
			sprintf(format,"%%%u[^/]",topLen);
			sscanf(url+8,format,urlTop);
			len=strlen(urlTop);
			sprintf(format,"%%%us",endLen);
			sscanf(url+len+8,format,urlEnd);
			return;
		}
		else
		{
			sprintf(format,"%%%u[^/]",topLen);
			sscanf(url,format,urlTop);
			len=strlen(urlTop);
			sprintf(format,"%%%us",endLen);
			sscanf(url+len,format,urlEnd);
			return;
		}
	}
	else
	{
		sprintf(format,"%%%u[^/]",topLen);
		sscanf(url+7,format,urlTop);
		len=strlen(urlTop);
		sprintf(format,"%%%us",endLen);
		sscanf(url+len+7,format,urlEnd);
	}
}
bool LogSystem::recordFileError(const char* fileName)
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
Json::Json()
{
	buffer=NULL;
	text=NULL;
	error=NULL;
	nowLen=0;
	maxLen=0;
	memset(this->word,0,sizeof(char)*30);
}
Json::Json(const char* jsonText)
{
	text=jsonText;
	buffer=NULL;
	nowLen=0;
	maxLen=0;
	memset(this->word,0,sizeof(char)*30);		
}
Json::~Json()
{
	if(this->buffer!=NULL)
		free(buffer);
}
bool Json::init(unsigned int bufferLen)
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
int Json::httpJsonCreate(void* buffer,unsigned int buffLen)
{
	if(buffLen<this->nowLen+100)
		return -1;
	sprintf((char*)buffer,"HTTP/1.1 200 OK\r\n"
		"Server LCserver/1.1\r\n"
		"Connection: keep-alive\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %ld\r\n\r\n"
		"%s",strlen(this->buffer),this->buffer);
	return strlen((char*)buffer);
}
bool Json::addKeyValue(const char* key,const char* value)
{
	char temp[200]={0};
	if(key==NULL||value==NULL)
	{
		error="key or value NULL";
		return false;
	}
	if(nowLen+strlen(key)+strlen(value)>maxLen)
	{
		error="buffer too short";
		return false;
	}
	if(strlen(key)+strlen(value)>=180)
	{
		error="buffer too short";
		return false;
	}
	if(buffer[strlen(buffer)-1]=='}')
		buffer[strlen(buffer)-1]=',';
	int len=sprintf(temp,"\"%s\":\"%s\"}",key,value);
	strcat(this->buffer,temp);
	nowLen+=len;
	return true;
}
bool Json::addKeyValInt(const char* key,int value)
{
	char temp[50]={0};
	if(key==NULL)
	{
		error="key is NULL";
		return false;
	}
	if(nowLen+50>maxLen)
	{
		error="buffer too short";
		return false;
	}
	if(strlen(key)>=45)
	{
		error="buffer too short";
		return false;	
	}
	if(buffer[strlen(buffer)-1]=='}')
		buffer[strlen(buffer)-1]=',';
	int len=sprintf(temp,"\"%s\":%d}",key,value);
	strcat(this->buffer,temp);
	nowLen+=len;
	return true;	
}
bool Json::addKeyValFloat(const char* key,float value,int output)
{
	char temp[70]={0};
	if(nowLen+50>maxLen)
	{
		error="buffer too short";
		return false;
	}
	if(NULL==key)
	{
		error="key is NULL";
		return false;
	}
	if(strlen(key)>=45)
	{
		error="buffer too short";
		return false;
	}
	if(buffer[strlen(buffer)-1]=='}')
		buffer[strlen(buffer)-1]=',';
	int len=sprintf(temp,"\"%s\":%.*f}",key,output,value);
	strcat(this->buffer,temp);
	nowLen+=len;
	return true;		
}
bool Json::jsonToFile(const char* fileName)
{
	FILE* fp=fopen(fileName,"w+");
	if(fp==NULL)
		return false;
	fprintf(fp,"%s",this->buffer);
	fclose(fp);
	return true;
}
const char* Json::operator[](const char* key)
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
float Json::getValueFloat(const char* key,bool& flag)
{
	float value=0;
	if(key==NULL)
		return -1;
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
int Json::getValueInt(const char* key,bool& flag)
{
	int value=0;
	if(key==NULL)
		return -1;
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
void Json::addOBject(const Object& obj)
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
			if(obj.key!=NULL)
				strcat(this->buffer,obj.key);
			strcat(this->buffer,"\":{");
			for(unsigned int i=0;i<obj.arrLen;i++)
				this->addOBject(obj.pobj[0]);
			strcat(this->buffer,"}");
			break;
	}
}
bool Json::addKeyObj(const char* key,const char* value)
{
	char temp[1000]={0};
	if(key==NULL||value==NULL)
	{
		error="key or value NULL";
		return false;
	}
	if(nowLen+strlen(key)+strlen(value)>maxLen)
	{
		error="buffer too short";
		return false;
	}
	if(strlen(key)+strlen(value)>=980)
	{
		error="buffer too short";
		return false;
	}
	if(buffer[strlen(buffer)-1]=='}')
		buffer[strlen(buffer)-1]=',';
	int len=sprintf(temp,"\"%s\":%s}",key,value);
	strcat(this->buffer,temp);
	nowLen+=len;
	return true;		
}
bool Json::addArray(TypeJson type,const char* key,void** array,unsigned int arrLen,unsigned int floatNum)
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
				if((char*)array[i]!=NULL)
				{
					strcat(buffer,(char*)array[i]);
					len+=strlen((char*)array[i]);
				}
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
void Json::createObject(char* pbuffer,int bufferLen,const Object& obj)
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
			if(obj.key!=NULL)
				strcat(this->buffer,obj.key);
			strcat(this->buffer,"\":{");
			for(unsigned int i=0;i<obj.arrLen;i++)
				this->addOBject(obj.pobj[0]);
			strcat(this->buffer,"}");
			break;
	}	
}
int Json::createObjInt(char* pbuffer,unsigned int bufferLen,const char* key,int value)
{
	if(pbuffer==NULL||key==NULL)
	{
		error="buffer or key NULL";
		return -1;
	}
	if(pbuffer[strlen(pbuffer)-1]=='}')
		pbuffer[strlen(pbuffer)-1]=',';
	if(strlen(pbuffer)==0)
		strcat(pbuffer,"{");
	if(bufferLen<strlen(pbuffer)+strlen(key))
	{
		error="buffer is too short";
		return -1;
	}
	char temp[100]={0};
	int len=sprintf(temp,"\"%s\":%d}",key,value);
	strcat(pbuffer,temp);
	len=strlen((char*)pbuffer);
	return len;
}
int Json::createObjFloat(char* pbuffer,unsigned int bufferLen,const char* key,float value,int output)
{
	if(pbuffer==NULL||key==NULL)
	{
		error="buffer or key NULL";
		return -1;
	}
	if(pbuffer[strlen(pbuffer)-1]=='}')
		pbuffer[strlen(pbuffer)-1]=',';
	if(strlen(pbuffer)==0)
		strcat(pbuffer,"{");
	if(bufferLen<strlen(pbuffer)+strlen(key))
	{
		error="buffer is too short";
		return -1;
	}
	char temp[100]={0};
	int len=sprintf(temp,"\"%s\":%.*f}",key,output,value);
	strcat(pbuffer,temp);
	len=strlen((char*)pbuffer);
	return len;
}
int Json::createObjValue(char* pbuffer,unsigned int bufferLen,const char* key,const char* value)
{
	if(pbuffer==NULL||key==NULL||value==NULL)
	{
		error="buffer or key NULL";
		return -1;
	}
	char temp[200]={0};
	if(strlen(pbuffer)+strlen(key)+strlen(value)>bufferLen)
	{
		error="buffer is too short";
		return -1;
	}
	if(strlen(key)+strlen(value)>=180)
	{
		error="buffer is too short";
		return -1;
	}
	if(pbuffer[strlen(pbuffer)-1]=='}')
		pbuffer[strlen(pbuffer)-1]=',';
	if(strlen(pbuffer)==0)
		strcat(pbuffer,"{");
	int len=sprintf(temp,"\"%s\":\"%s\"}",key,value);
	strcat(pbuffer,temp);
	len=strlen((char*)pbuffer);
	return len;
}
bool Json::createObjArray(char* pbuffer,unsigned int bufferLen,TypeJson type,const char* key,void** array,unsigned int arrLen,unsigned int floatNum)
{
	char temp[200]={0};
	if(array==NULL||arrLen==0||pbuffer==NULL)
	{
		error="buffer is NULL";
		return false;
	}
	if(strlen(pbuffer)+strlen(key)>bufferLen)
	{
		error="buffer is too short";
		return false;
	}
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
int Json::createObjObj(char* pbuffer,unsigned int bufferLen,const char* key,const char* value)
{
	char temp[500]={0};
	if(pbuffer==NULL||key==NULL||value==NULL)
	{
		error="pbuffer NULL or key NULL or value NULL";
		return -1;
	}
	if(strlen(pbuffer)+strlen(key)+strlen(value)>bufferLen)
	{
		error="buffer is too short";
		return -1;
	}
	if(strlen(key)+strlen(value)>=490)
	{
		error="buffer is too short";
		return -1;
	}
	if(pbuffer[strlen(pbuffer)-1]=='}')
		pbuffer[strlen(pbuffer)-1]=',';
	if(strlen(pbuffer)==0)
		strcat(pbuffer,"{");
	int len=sprintf(temp,"\"%s\":%s}",key,value);
	strcat(pbuffer,temp);
	nowLen+=len;
	len=strlen((char*)pbuffer);
	return len;
}
WebToken::WebToken()
{
	backString=NULL;
	memset(err,0,sizeof(char)*30);
	sprintf(err,"no error");
}
const char* WebToken::createToken(const char* key,const char* encryption,char* getString,unsigned int stringLen,unsigned int liveSecond)
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
	char tempString[50]={0},endString[30]={0};
	sprintf(endString,"%d",end);
	for(unsigned int i=0;i<strlen(endString);i++)
		endString[i]+=50;
	sprintf(tempString,".%s.%c",endString,encryption[0]);
	strcat(backString,tempString);
	strcpy(getString,backString);
	return getString;
}
const char* WebToken::decryptToken(const char* key,const char* token,char* buffer,unsigned int bufferLen)
{
	char* temp=strchr((char*)token,'.');
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
	if(sscanf(temp+1,"%20[^.]",endString)<=0)
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
FileGet::FileGet()
{
	pbuffer=NULL;
}
FileGet::~FileGet()
{
	if(pbuffer!=NULL)
	{
		free(pbuffer);
		pbuffer=NULL;
	}
}
int FileGet::getFileLen(const char* fileName)
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
bool FileGet::getFileMsg(const char* fileName,char* buffer,unsigned int bufferLen)
{
	unsigned int i=0,len=0;
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
bool FileGet::fileStrstr(const char* fileName,const char* strFind)
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
bool FileGet::writeToFile(const char* fileName,const char* buffer,unsigned int writeLen)
{
	FILE* fp=fopen(fileName,"wb+");
	if(fp==NULL)
		return false;
	for(unsigned int i=0;i<writeLen;i++)
		fputc(buffer[i],fp);
	fclose(fp);
	return true;
}
}