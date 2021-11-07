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
		case JSON:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:application/json\r\n"
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
const char* DealHttp::getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen)
{
	char* temp=strstr((char*)message,ask);
	if(temp==NULL)
		return NULL;
	sscanf(temp+strlen(askWay)+1,"%s",buffer);
	return buffer;
}
const char* DealHttp::getRouteValue(const void* routeMeg,const char* key,char* value,unsigned int valueLen)
{
	char* temp=strstr((char*)routeMeg,key);
	if(temp==NULL)
		return NULL;
	return this->findBackString(temp,strlen(key),value,valueLen);
}
const char* DealHttp::getWildUrl(const void* getText,const char* route,char* buffer,int maxLen)
{
	char* temp=strstr((char*)getText,route);
	if(temp==NULL)
		return NULL;
	temp+=strlen(route);
	sscanf(temp,"%s",buffer);
}
void DealHttp::dealUrl(const char* url,char* urlTop,char* urlEnd)
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
bool Json::jsonInit(unsigned int bufferLen)
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
bool Json::addKeyValue(const char* key,const char* value)
{
	char temp[200]={0};
	if(nowLen+strlen(key)+strlen(value)>maxLen)
		return false;
	if(strlen(key)+strlen(value)>=180)
		return false;
	int len=sprintf(temp,"\"%s\":\"%s\";",key,value);
	strcat(this->buffer,temp);
	nowLen+=len;
	return true;
}
bool Json::addKeyValInt(const char* key,int value)
{
	char temp[50]={0};
	if(nowLen+50>maxLen)
		return false;
	if(strlen(key)>=45)
		return false;	
	int len=sprintf(temp,"\"%s\":%d;",key,value);
	strcat(this->buffer,temp);
	nowLen+=len;
	return true;	
}
bool Json::addKeyValFloat(const char* key,float value,int output)
{
	char temp[50]={0};
	if(nowLen+50>maxLen)
		return false;
	if(strlen(key)>=45)
		return false;	
	int len=sprintf(temp,"\"%s\":%.*f;",key,output,value);
	strcat(this->buffer,temp);
	nowLen+=len;
	return true;		
}
const char* Json::endJson()
{
	if(nowLen+5>maxLen)
		return NULL;
	strcat(buffer,"}");
	return this->buffer;
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
	char tempString[30]={0};
	sprintf(tempString,"&%d&%c",end,encryption[0]);
	strcat(backString,tempString);
	strcpy(getString,backString);
	return getString;
}
const char* WebToken::decryptToken(const char* key,const char* token,char* buffer,unsigned int bufferLen)
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
	int end=0;
	if(sscanf(temp+1,"%d",&end)<=0)
	{
		sprintf(err,"get time wrong");
		return NULL;
	}
	int keyTemp=0;
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
