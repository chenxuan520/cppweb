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
protected:
    int* pfdn;//pointer if file descriptor
    int fdNumNow;//num of fd now
    int fdMax;//fd max num
    bool addFd(int addsoc);
    bool deleteFd(int clisoc);
public:
    ServerTcpIp(unsigned short port=5200,int epollNum=1,int wait=5);
    ~ServerTcpIp();//clean server
    bool bondhost();//bond myself first
	bool setlisten();//set listem to accept second
	int acceptClient();//wait until success model one
	bool acceptClients(int* pcliNum);//model two
	int receiveOne(void* pget,int len);//model one
	int receiveSocket(int clisoc,void* pget,int len);
	int sendClientOne(const void* psen,int len);//model one
	void sendEverySocket(void* psen,int len);
	int sendSocket(int socCli,const void* psen,int len);//send by socket
	bool selectModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int,void* ,void*,ServerTcpIp& ));
	bool updateSocket(int* array,int* pcount);//get epoll array
	bool findSocket(int cliSoc);//find if socket is connect
	char* getHostName();//get self name
	char* getHostIp();//get self ip
	char* getPeerIp(int cliSoc,int* pcliPort);//get ip and port by socket
	bool epollModel(int* pthing,int* pnum,void* pget,int len,void* pneed,int (*pfunc)(int ,int ,int ,void* ,void*,ServerTcpIp& ));
	bool disconnectSocket(int clisock);//disconnect from socket
};
#endif
