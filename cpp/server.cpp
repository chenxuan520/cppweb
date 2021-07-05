/********************************
	author:chenxuan
	date:2021.7.5
	funtion:do not change this file 
*********************************/
#include"../hpp/server.h"
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<iostream>
using namespace std;
ServerTcpIp::ServerTcpIp(unsigned short port,int epollNum,int wait,int maxClient)
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
ServerTcpIp::~ServerTcpIp()//clean server
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
bool ServerTcpIp::bondhost()//bond myself first
{
    if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
        return false;
    return true;
}
bool ServerTcpIp::setlisten()//set listem to accept second
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
bool ServerTcpIp::acceptClient()//wait until success model one
{
    sockC=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
    return true;
}
bool ServerTcpIp::acceptClientsModelTwo(int cliNum)//model two
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
bool ServerTcpIp::receiveMystl(void* pget,int len)//model one
{
    if(recv(sockC,(char*)pget,len,0)==-1)
        return false;
    return true;
}
bool ServerTcpIp::receiveSMystlModelTwo(void* prec,int cliNum,int len)//model two
{
    if(recv(psockClients[cliNum],(char*)prec,len,0)==-1)
    {
        psockClients[cliNum]=0;
        return false;
    }
    return true;
}
bool ServerTcpIp::sendClientMystl(const void* ps,int len)//model one
{
    if(send(sockC,(char*)ps,len,0)==-1)
        return false;
    return true;
}
bool ServerTcpIp::sendClientSMystlModelTwo(const void* ps,int cliNum,int len)//model two
{
    if(send(psockClients[cliNum],(char*)ps,len,0)==-1)
        return false;
    return true;
}
bool ServerTcpIp::sendClientsEveryoneMystlTwo(const void* ps,int len)//model two
{
    for(int i=0;i<numClient;i++)
    { 
        if(psockClients[i]==0)
            continue;
        send(psockClients[i],(char*)ps,len,0);
    }
    return true;
}
bool ServerTcpIp::selectModelMysql(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,void* ,void*,ServerTcpIp& ))
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
bool ServerTcpIp::selectSendMystl(const void* ps,int cliNum,int len)
{
    int clientSent=fdClients.fds_bits[cliNum];
    if(send(fdClients.fds_bits[cliNum],(char*)ps,len,0)==-1)
        return false;
    return true;
}
bool ServerTcpIp::selectSendEveryoneMystl(void* ps,int len)
{
    for(int i=0;i<fd_count;i++)
    { 
        int clientSent=fdClients.fds_bits[i];
        if(clientSent!=0)
            send(fdClients.fds_bits[i],(char*)ps,len,0);
    }
    return true;
}
bool ServerTcpIp::updateSocketSelect(int* p,int* pcount)
{
    if(fd_count!=0)
        *pcount=fd_count;
    else
        return false;
    for(int i=0;i<fd_count;i++)
        p[i]=fdClients.fds_bits[i];
    return true;
}
bool ServerTcpIp::updateSocketEpoll(int* p,int* pcount)
{
    if(fdNumNow!=0)
        *pcount=fdNumNow;
    else
        return false;
    for(int i=0;i<fdNumNow;i++)
        p[i]=pfdn[i];
    return true;
}
bool ServerTcpIp::sendSocketMystlSelect(int toClient,const void* ps,int len)
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
bool ServerTcpIp::sendEverySocket(void* ps,int len)
{
    for(int i=0;i<fdNumNow;i++)
    {
        if(pfdn[i]!=0)
            send(pfdn[i],ps,len,0);
    }
    return true;
}
bool ServerTcpIp::sendSocketAll(int socCli,const void* ps,int len)
{
    if(send(socCli,(char*)ps,len,0)==-1)
        return false;
    return true;
}
int ServerTcpIp::findSocketSelsct(int i)
{
    if(fdClients.fds_bits[i]!=0)
        return fdClients.fds_bits[i];
    else
        return -1;
}
bool ServerTcpIp::findSocketEpoll(int cliSoc)
{
    for(int i=0;i<fdNumNow;i++)
    {
        if(pfdn[i]==cliSoc)
            return true;
    }
    return false;
}
char* ServerTcpIp::getHostName()
{
    char name[30]={0};
    gethostname(name,30);
    memcpy(hostname,name,30);
    return hostname;
}
char* ServerTcpIp::getHostIp()
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
bool ServerTcpIp::epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,void* ,void*,ServerTcpIp& ))
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
bool ServerTcpIp::addFd(int addsoc)
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
bool ServerTcpIp::deleteFd(int clisoc)
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
