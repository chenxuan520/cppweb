#ifndef _SERVER_H_
#define _SERVER_H_
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
class ServerTcpIp{
private:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now; 
	int max;//the most clients;
	int fd_count;//sum of clients in fd_set
	int epfd;//file descriptor to ctrl epoll
	char* hostip;//host IP 
	char* hostname;//host name
	int sock;//file descriptor of host;
	int* psockClients;//client[];
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
    bool addFd(int addsoc);
    bool deleteFd(int clisoc);
public:
    ServerTcpIp(unsigned short port=5200,int epollNum=0,int wait=5,int maxClient=0);
    ~ServerTcpIp();//clean server
    bool bondhost();//bond myself first
    bool setlisten();//set listem to accept second
    bool acceptClient();//wait until success model one
    bool acceptClientsModelTwo(int cliNum);//model two
    bool receiveMystl(void* pget,int len);//model one
    bool receiveSMystlModelTwo(void* prec,int cliNum,int len);//model two
    bool sendClientMystl(const void* ps,int len);//model one
	bool sendClientSMystlModelTwo(const void* ps,int cliNum,int len);//model two;
	bool sendClientsEveryoneMystlTwo(const void* ps,int len);//model two
	bool selectModelMysql(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,void* ,void*,ServerTcpIp& ));
	bool selectSendMystl(const void* ps,int cliNum,int len);
	bool selectSendEveryoneMystl(void* ps,int len);
	bool updateSocketSelect(int* p,int* pcount);
	bool updateSocketEpoll(int* p,int* pcount);
	bool sendSocketMystlSelect(int toClient,const void* ps,int len);
    bool sendEverySocket(void* ps,int len);
	bool sendSocketAll(int socCli,const void* ps,int len);
	int findSocketSelsct(int i);
	bool findSocketEpoll(int cliSoc);
	char* getHostName();
	char* getHostIp();
	bool epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,void* ,void*,ServerTcpIp& ));
};
#endif