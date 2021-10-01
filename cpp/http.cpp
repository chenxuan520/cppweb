#include"../hpp/http.h"
#include<iostream>
#include<string.h>
using namespace std;
DealHttp::DealHttp()
{
	for(int i=0;i<256;i++)
		ask[i]=0;
	pfind=NULL;
}
bool DealHttp::cutLineAsk(char* pask,const char* pcutIn)
{
	char* ptemp=strstr(pask,pcutIn);
	if(ptemp==NULL)
		return false;
	while(*(ptemp++)!='\n');
	*ptemp=0;
	return true;
}
const char* DealHttp::analysisHttpAsk(void* pask,const char* pneed,int needLen)
{
	pfind=strstr((char*)pask,pneed);
	if(pfind==NULL)
		return NULL;
	return this->findBackString(pfind,needLen,ask,256);
}
char* DealHttp::findBackString(char* local,int len,char* word,int maxWordLen)
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
void DealHttp::createTop(FileKind kind,char* ptop,int* topLen,int fileLen)//1:http 2:down 3:pic
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
bool DealHttp::createSendMsg(FileKind kind,char* pask,const char* pfile,int* plong)
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
char* DealHttp::findFileMsg(const char* pname,int* plen,char* buffer)
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
int DealHttp::autoAnalysisGet(const char* pask,char* psend,const char* pfirstFile,int* plen)
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
const char* DealHttp::getKeyValue(const void* message,const char* key,char* value,int maxValueLen)
{
	char* temp=strstr((char*)message,key);
	if(temp==NULL)
		return NULL;
	return this->findBackString(temp,strlen(key),value,maxValueLen);
}
const char* DealHttp::getKeyLine(const void* message,const char* key,char* line,int maxLineLen)
{
	int i=0;
	char* ptemp=strstr((char*)message,key);
	if(ptemp==NULL)
		return NULL;
	ptemp+=strlen(key);
	while(*(ptemp++)!='\n'&&i<maxLineLen)
		line[i++]=*ptemp;
	line[i]=0;
	return line;
}
bool  DealAttack::dealAttack(int isUpdate,int socketCli,int maxTime)//check if accket
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
bool DealAttack::attackLog(int port,const char* ip,const char* pfileName)//log accket
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
