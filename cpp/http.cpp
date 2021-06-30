#include"../hpp/http.h"
#include<iostream>
#include<string.h>
using namespace std;
DealHttp::DealHttp()
{
    for(int i=0;i<256;i++)
        ask[i]=0;
    pfind=NULL;
    pfile=NULL;
}
DealHttp::~DealHttp()
{
    if(pfile!=NULL)
        free(pfile);
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
    return this->findBackString(pfind,needLen,ask);
}
char* DealHttp::findBackString(char* ps,int len,char* word)
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
void DealHttp::createTop(int kind,char* ptop,int* topLen,int fileLen)//1:http 2:down 3:pic
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
    }
}
bool DealHttp::createSendMsg(int kind,char* pask,const char* pfile,int* plong)
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
        memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);
        break;
    case 6:
        len=this->getFileLen(pfile);
        if(len==0)
            return false;
        this->createTop(6,pask,&temp,len);
        memcpy(pask+temp,this->findFileMsg(pfile,&noUse),len+3);
        break;
    default:
        break;
    }
    *plong=len+temp+10;
    return true;
}
char* DealHttp::findFileMsg(const char* pname,int* plen)
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
    if(strcmp(ask,"HTTP/1.1")==0)
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
