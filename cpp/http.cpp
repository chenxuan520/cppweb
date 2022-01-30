#include"../hpp/http.h"
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<stdio.h>
#include<stack>
#include<stdarg.h>
#include<string>
#include<vector>
#include<unordered_map>
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
	error=NULL;
	obj=NULL;
	text=NULL;
	maxLen=256;
	floNum=3;
}
Json::Json(const char* jsonText)
{
	error=NULL;
	obj=NULL;
	maxLen=256;
	floNum=3;
	if(jsonText==NULL||strlen(jsonText)==0)
	{
		error="message error";
		return;
	}
	text=(char*)malloc(strlen(jsonText)+10);
	if(text==NULL)
	{
		error="malloc wrong";
		return;
	}
	memset(text,0,strlen(jsonText)+10);
	strcpy(text,jsonText);
	deleteComment();
	deleteSpace();
	if(false==pairBracket())
	{
		error="pair bracket wrong";
		return;
	}
	if(text[0]!='{')
	{
		error="text wrong";
		return;
	}
	obj=analyseObj(text,bracket[text]);
	if(obj==NULL)
	{
		error="malloc wrong";
		return;
	}
}
Json::~Json()
{
	deleteNode(obj);
	if(text!=NULL)
		free(text);
	for(auto iter=memory.begin();iter!=memory.end();iter++)
		free(iter->first);
}
const char* Json::formatPrint(const Json::Object* exmaple,unsigned buffLen)
{
	char* buffer=(char*)malloc(sizeof(char)*buffLen);
	if(buffer==NULL)
	{
		error="malloc wrong";
		return NULL;
	}
	memset(buffer,0,sizeof(char)*buffLen);
	memory.insert(std::pair<char*,unsigned>{buffer,buffLen});
	printObj(buffer,exmaple);
	return buffer;
}
Json::Object* Json::operator[](const char* key)
{
	if(hashMap.find(std::string(key))==hashMap.end())
		return NULL;
	return hashMap.find(std::string(key))->second;
}
bool Json::addKeyVal(char* obj,TypeJson type,const char* key,...)
{
	if(obj==NULL)
	{
		error="null buffer";
		return false;
	}
	va_list args;
	va_start(args,key);
	if(obj[strlen(obj)-1]=='}')
	{
		if(obj[strlen(obj)-2]!='{')
			obj[strlen(obj)-1]=',';
		else
			obj[strlen(obj)-1]=0;
	}
	if(memory.find(obj)==memory.end())
	{
		error="wrong object";
		return false;
	}
	if(memory[obj]-strlen(obj)<strlen(key)+4)
	{
		error="obj too short";
		return false;
	}
	sprintf(obj,"%s\"%s\":",obj,key);
	int valInt=0;
	char* valStr=NULL;
	float valFlo=9;
	bool valBool=false;
	switch(type)
	{
	case INT:
		valInt=va_arg(args,int);
		if(memory[obj]-strlen(obj)<15)
		{
			error="obj too short";
			return false;
		}
		sprintf(obj,"%s%d",obj,valInt);
		break;
	case FLOAT:
		valFlo=va_arg(args,double);
		if(memory[obj]-strlen(obj)<15)
		{
			error="obj too short";
			return false;
		}
		sprintf(obj,"%s%.*f",obj,floNum,valFlo);
		break;
	case STRING:
		valStr=va_arg(args,char*);
		if(valStr==NULL)
		{
			error="null input";
			return false;
		}
		if(memory[obj]-strlen(obj)<strlen(obj)+5)
		{
			error="obj too short";
			return false;
		}
		sprintf(obj,"%s\"%s\"",obj,valStr);
		break;
	case EMPTY:
		if(memory[obj]-strlen(obj)<5)
		{
			error="obj too short";
			return false;
		}
		strcat(obj,"null");
		break;
	case BOOL:
		valBool=va_arg(args,int);
		if(memory[obj]-strlen(obj)<5)
		{
			error="obj too short";
			return false;
		}
		if(valBool==true)
			strcat(obj,"true");
		else
			strcat(obj,"false");
		break;
	case OBJ:
	case ARRAY:
		valStr=va_arg(args,char*);
		if(memory[obj]-strlen(obj)<strlen(obj)+5)
		{
			error="obj too short";
			return false;
		}
		if(valStr==NULL)
		{
			error="null input";
			return false;
		}
		sprintf(obj,"%s%s",obj,valStr);
		break;
	default:
		error="can not insert this type";
		strcat(obj,"}");
		return false;
	}
	strcat(obj,"}");
	return true;
}
char* Json::createObject(unsigned maxBuffLen)
{
	char* now=(char*)malloc(sizeof(char)*maxBuffLen);
	if(now==NULL||maxBuffLen<4)
	{
		error="init worng";
		return NULL;
	}
	else
		memory.insert(std::pair<char*,int>{now,maxBuffLen});
	memset(now,0,sizeof(char)*maxBuffLen);
	strcpy(now,"{}");
	return now;
}
char* Json::createArray(unsigned maxBuffLen,TypeJson type,unsigned arrLen,void* arr)
{
	if(arr==NULL||maxBuffLen<4)
	{
		error="null input";
		return NULL;
	}
	char* now=(char*)malloc(sizeof(char)*maxBuffLen);
	if(now==NULL)
	{
		error="malloc worng";
		return NULL;
	}
	else
		memory.insert(std::pair<char*,int>{now,maxBuffLen});
	memset(now,0,sizeof(char)*maxBuffLen);
	strcat(now,"[");
	int* arrInt=(int*)arr;
	float* arrFlo=(float*)arr;
	char** arrStr=(char**)arr;
	bool* arrBool=(bool*)arr;
	unsigned i=0;
	switch(type)
	{
	case INT:
		for(i=0;i<arrLen;i++)
		{
			if(maxBuffLen-strlen(now)<std::to_string(arrInt[i]).size()+3)
			{
				error="bufferLen is too small";
				return NULL;
			}
			sprintf(now,"%s%d,",now,arrInt[i]);
		}
		break;
	case FLOAT:
		for(i=0;i<arrLen;i++)
		{
			if(maxBuffLen-strlen(now)<std::to_string(arrFlo[i]).size()+3)
			{
				error="bufferLen is too small";
				return NULL;
			}
			sprintf(now,"%s%.*f,",now,floNum,arrFlo[i]);
		}
		break;
	case STRING:
		for(i=0;i<arrLen;i++)
		{
			if(maxBuffLen-strlen(now)<strlen(arrStr[i])+5)
			{
				error="bufferLen is too small";
				return NULL;
			}
			sprintf(now,"%s\"%s\",",now,arrStr[i]);
		}
		break;
	case OBJ:
	case ARRAY:
		for(i=0;i<arrLen;i++)
		{
			if(maxBuffLen-strlen(now)<strlen(arrStr[i])+4)
			{
				error="bufferLen is too small";
				return NULL;
			}
			sprintf(now,"%s%s,",now,arrStr[i]);
		}
		break;
	case BOOL:
		for(i=0;i<arrLen;i++)
		{
			if(maxBuffLen-strlen(now)<6)
			{
				error="bufferLen is too small";
				return NULL;
			}
			if(arrBool)
				strcat(now,"true,");
			else
				strcat(now,"false,");
		}
		break;
	default:
		error="struct cannot be a array";
		break;
	}
	if(now[strlen(now)-1]==',')
		now[strlen(now)-1]=']';
	else
		strcat(now,"]");
	now[strlen(now)]=0;
	return now;
}
Json::Object* Json::analyseObj(char* begin,char* end)
{
	Object * root=new Object,*last=root;
	root->type=STRUCT;
	char* now=begin+1,*next=now;
	char* word=(char*)malloc(sizeof(char)*maxLen),*val=(char*)malloc(sizeof(char)*maxLen),temp=*end;
	if(word==NULL||val==NULL)
	{
		error="malloc wrong";
		return NULL;
	}
	memset(word,0,sizeof(char)*maxLen);
	memset(val,0,sizeof(char)*maxLen);
	*end=0;
	while(now<end)
	{
		Object* nextObj=new Object;
		memset(word,0,sizeof(char)*maxLen);
		findString(now,word,maxLen);
		nextObj->key=word;
		hashMap.insert(std::pair<std::string,Object*>{word,nextObj});
		now+=strlen(word)+3;
		if(*now=='\"')
		{
			nextObj->type=STRING;
			next=strchr(now+1,'\"');
			while(next!=NULL&&*(next-1)=='\\')
				next=strchr(next+1,'\"');
			if(next==NULL)
			{
				error="string wrong";
				return NULL;
			}
			for(unsigned i=0;now+i+1<next;i++)
				val[i]=*(now+i+1);
			val[strlen(val)]=0;
			nextObj->strVal=val;
			now=next+1;
			if(*now==',')
				now++;
		}
		else if(('0'<=*now&&'9'>=*now)||*now=='-')
		{
			next=now;
			nextObj->type=INT;
			while(*next!=','&&*next!=0)
			{
				next++;
				if(*next=='.')
					nextObj->type=FLOAT;
			}
			if(nextObj->type==INT)
				sscanf(now,"%d",&nextObj->intVal);
			else
				sscanf(now,"%f",&nextObj->floVal);
			now=next+1;
		}
		else if(*now=='[')
		{
			next=bracket[now];
			if(next==NULL)
			{
				error="format wrong";
				return root;
			}
			nextObj->type=ARRAY;
			nextObj->arrType=analyseArray(now,next,nextObj->arr);
			now=next+1;
			if(*now==',')
				now++;
		}
		else if(*now=='{')
		{
			next=bracket[now];
			if(next==NULL)
			{
				error="format wrong";
				return root;
			}
			nextObj->type=OBJ;
			nextObj->objVal=analyseObj(now,next);
			now=next+1;
			if(*now==',')
				now++;
		}
		else if(strncmp(now,"true",4)==0)
		{
			nextObj->type=BOOL;
			now+=4;
			if(*now==',')
				now++;
			nextObj->boolVal=true;
		}
		else if(strncmp(now,"false",5)==0)
		{
			nextObj->type=BOOL;
			now+=5;
			if(*now==',')
				now++;
			nextObj->boolVal=false;
		}
		else if(strncmp(now,"null",4)==0)
		{
			nextObj->type=EMPTY;
			now+=4;
			if(*now==',')
				now++;
		}
		else
		{
			error="text wrong";
			free(word);
			free(val);
			return root;
		}
		last->nextObj=nextObj;
		last=nextObj;
	}
	*end=temp;
	free(word);
	free(val);
	return root;
}
void Json::deleteComment()
{
	unsigned flag=0;
	for(unsigned i=0;i<strlen(text);i++)
	{
		if(text[i]=='\"'&&text[i-1]!='\\')
			flag++;
		else if(flag%2==0&&text[i]=='/'&&i+1<strlen(text)&&text[i+1]=='/')
		{
			while(text[i]!='\n'&&i<strlen(text))
			{
				text[i]=' ';
				i++;
			}
		}
		else if(flag%2==0&&text[i]=='/'&&i+1<strlen(text)&&text[i+1]=='*')
		{
			while(i+1<strlen(text))
			{
				if(text[i+1]=='/'&&text[i]=='*')
				{
					text[i]=' ';
					text[i+1]=' ';
					break;
				}
				text[i]=' ';
				i++;
			}
		}
		else 
			continue;
	}
}
Json::TypeJson Json::analyseArray(char* begin,char* end,std::vector<Object*>& array)
{
	char* now=begin+1,*next=end,*word=(char*)malloc(sizeof(char)*maxLen);
	if(word==NULL)
	{
		error="malloc wrong";
		return INT;
	}
	memset(word,0,sizeof(char)*maxLen);
	Object* nextObj=NULL;
	if(('0'<=*now&&'9'>=*now)||*now=='-')
	{
		next=now;
		while(next<end&&*next!=',')
			next++;
		TypeJson type=judgeNum(now,next);
		while(now<end&&now!=NULL)
		{
			nextObj=new Object;
			nextObj->isData=true;
			nextObj->type=type;
			if(nextObj->type==INT)
			{
				findNum(now,type,&nextObj->intVal);
				array.push_back(nextObj);
			}
			else
			{
				findNum(now,type,&nextObj->floVal);
				array.push_back(nextObj);
			}
			now=strchr(now+1,',');
			if(now!=NULL)
				now++;
		}
		nextObj->arrType=type;
	}
	else if(*now=='\"')
	{
		while(now<end&&now!=NULL)
		{
			findString(now,word,maxLen);
			nextObj=new Object;
			nextObj->type=STRING;
			nextObj->isData=true;
			nextObj->strVal=word;
			array.push_back(nextObj);
			now=strchr(now+1,',');
			if(now==NULL)
				break;
			now+=1;
		}
	}
	else if(strncmp(now,"true",4)==0||strncmp(now,"false",5)==0)
	{
		while(now<end&&now!=NULL)
		{
			nextObj=new Object;
			nextObj->type=BOOL;
			nextObj->isData=true;
			nextObj->boolVal=strncmp(now,"true",4)==0;
			array.push_back(nextObj);
			now=strchr(now+1,',');
			if(now==NULL)
				break;
			now+=1;
		}
	}
	else if(*now=='{')
	{
		while(now<end&&now!=NULL)
		{
			next=bracket[now];
			nextObj=analyseObj(now,next);
			nextObj->type=OBJ;
			nextObj->isData=true;
			array.push_back(nextObj);
			now=next;
			now=strchr(now+1,',');
			if(now==NULL)
				break;
			now+=1;
		}
	}
	else if(*now=='[')
	{
		while(now<end&&now!=NULL)
		{
			next=bracket[now];
			nextObj=new Object;
			TypeJson type=analyseArray(now,next,nextObj->arr);
			nextObj->type=ARRAY;
			nextObj->arrType=type;
			nextObj->isData=true;
			array.push_back(nextObj);
			now=next;
			now=strchr(now+1,',');
			if(now==NULL)
				break;
			now+=1;
		}
	}
	else if(*now==']')
	{
		free(word);
		return INT;
	}
	else
	{
		error="array find wrong";
		free(word);
		return INT;
	}
	free(word);
	return nextObj->type;
}
void Json::findString(const char* begin,char* buffer,unsigned buffLen)
{
	const char* now=begin+1,*next=now;
	next=strchr(now+1,'\"');
	while(next!=NULL&&*(next-1)=='\\')
		next=strchr(next+1,'\"');
	for(unsigned i=0;now+i<next&&i<buffLen;i++)
		buffer[i]=*(now+i);
	buffer[strlen(buffer)]=0;
}
void Json::findNum(const char* begin,TypeJson type,void* pnum)
{
	if(type==INT)
	{
		if(sscanf(begin,"%d",(int*)pnum)<1)
			error="num wrong";
	}
	else
	{
		if(sscanf(begin,"%f",(float*)pnum)<1)
			error="num wrong";
	}
}
void Json::deleteSpace()
{
    unsigned j=0,k=0;
    unsigned flag=0;
    for(j=0,k=0; text[j]!='\0'; j++)
    {
    	if(text[j]!='\r'&&text[j]!='\n'&&text[j]!='\t'&&(text[j]!=' '||flag%2!=0))
    		text[k++]=text[j];
    	if(text[j]=='\"'&&j>0&&text[j-1]!='\\')
    		flag++;
    }
	text[k]=0;
}
void Json::deleteNode(Object* root)
{
	if(root==NULL)
		return;
	if(root->nextObj!=NULL)
		deleteNode(root->nextObj);
	if(root->arr.size()>0)
		for(unsigned i=0;i<root->arr.size();i++)
			deleteNode(root->arr[i]);
	if(root->objVal!=NULL)
		deleteNode(root->objVal);
	delete root;
	root=NULL;
}
bool Json::pairBracket()
{
	unsigned flag=0;
	std::stack<char*> sta;
	for(unsigned i=0;i<strlen(text);i++)
	{
		if((text[i]=='['||text[i]=='{')&&flag%2==0)
			sta.push(text+i);
		else if(text[i]==']'||text[i]=='}')
		{
			if(sta.empty())
				return false;
			if(text[i]==']'&&*sta.top()!='[')
				return false;
			if(text[i]=='}'&&*sta.top()!='{')
				return false;
			bracket.insert(std::pair<char*,char*>{sta.top(),&text[i]});
			sta.pop();
		}
		else if(text[i]=='\"'&&i>0&&text[i-1]!='\\')
			flag++;
		else
			continue;
	}
	if(!sta.empty())
		return false;
	return true;
}
bool Json::printObj(char* buffer,const Object* obj)
{
	unsigned deep=0;
	char* line=strrchr(buffer,'\n');
	if(line==NULL)
		deep=1;
	else
		deep=buffer+strlen(buffer)-line;
	strcat(buffer,"{\n");
	Object* now=obj->nextObj;
	while(now!=NULL)
	{
		for(unsigned i=0;i<deep+4;i++)
			strcat(buffer," ");
		switch(now->type)
		{
		case INT:
			sprintf(buffer,"%s\"%s\":%d,",buffer,now->key.c_str(),now->intVal);
			break;
		case FLOAT:
			sprintf(buffer,"%s\"%s\":%.*f,",buffer,now->key.c_str(),floNum,now->floVal);
			break;
		case STRING:
			sprintf(buffer,"%s\"%s\":\"%s\",",buffer,now->key.c_str(),now->strVal.c_str());
			break;
		case BOOL:
			if(now->boolVal)
				sprintf(buffer,"%s\"%s\":true,",buffer,now->key.c_str());
			else
				sprintf(buffer,"%s\"%s\":false,",buffer,now->key.c_str());
			break;
		case OBJ:
			sprintf(buffer,"%s\"%s\":",buffer,now->key.c_str());
			printObj(buffer,now->objVal);
			strcat(buffer,",");
			break;
		case ARRAY:
			sprintf(buffer,"%s\"%s\":",buffer,now->key.c_str());
			printArr(buffer,now->arrType,now->arr);
			strcat(buffer,",");
			break;
		case EMPTY:
			sprintf(buffer,"%s\"%s\":null",buffer,now->key.c_str());
			break;
		default:
			error="struct cannot print";
			return false;
		}
		strcat(buffer,"\n");
		now=now->nextObj;
		if(now==NULL)
			*strrchr(buffer,',')=' ';
	}
	for(unsigned i=0;i<deep-1;i++)
		strcat(buffer," ");
	strcat(buffer,"}");
	return true;
}
bool Json::printArr(char* buffer,TypeJson type,const std::vector<Object*>& arr)
{
	unsigned deep=0;
	char* line=strrchr(buffer,'\n');
	if(line==NULL)
		deep=0;
	else
		deep=buffer+strlen(buffer)-line;
	strcat(buffer,"[\n");
	for(unsigned i=0;i<arr.size();i++)
	{
		for(unsigned i=0;i<deep+4;i++)
			strcat(buffer," ");
		switch(type)
		{
		case INT:
			sprintf(buffer,"%s%d,",buffer,arr[i]->intVal);
			break;
		case FLOAT:
			sprintf(buffer,"%s%.*f,",buffer,floNum,arr[i]->floVal);
			break;
		case STRING:
			sprintf(buffer,"%s\"%s\",",buffer,arr[i]->strVal.c_str());
			break;
		case BOOL:
			if(arr[i]->boolVal)
				strcat(buffer,"true,");
			else
				strcat(buffer,"false,");
			break;
		case OBJ:
			printObj(buffer,arr[i]);
			strcat(buffer,",");
			break;
		case ARRAY:
			printArr(buffer,arr[i]->arrType,arr[i]->arr);
			strcat(buffer,",");
			break;
		default:
			error="struct cannot print";
			return false;
		}
		strcat(buffer,"\n");
		if(i==arr.size()-1)
			*strrchr(buffer,',')=' ';
	}
	for(unsigned i=0;i<deep-1;i++)
		strcat(buffer," ");
	strcat(buffer,"]");
	return true;
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
