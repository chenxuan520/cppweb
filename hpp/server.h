#ifndef _SERVER_H_
#define _SERVER_H_
#include"./http.h"
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
protected:
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
	char* error;
	unsigned int max;
	unsigned int now;
	bool isDebug;
public:
	HttpServer(unsigned port,bool debug=false);
	~HttpServer();
	bool routeHandle(AskType ask,RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
	const char* getWildUrl(const char* route,char* buffer,int maxLen);;
	void run(int memory,const char* defaultFile);
	int httpSend(int num,void* buffer,int sendLen);
	inline void* recText()
	{
		return this->getText;
	}
	inline const char* lastError()
	{
		return error;
	}
private:
	int func(int num,void* pget,void* sen,const char* defaultFile,HttpServer& server);
	void epollHttp(void* pget,int len,void* pneed,const char* defaultFile);
};
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
	Email(const char* domain,bool debug=false);
	bool emailSend(const char* sendEmail,const char* passwd,const char* recEmail,const char* body);
	char ConvertToBase64(char uc);
    void EncodeBase64(char *dbuf, char *buf128, int len);
    void CreateSend(const char* youName,const char* toName,const char* from,const char* to,const char* subject,const char* body,char* buf);
    inline const char* LastError()
    {
		return error;
	}
	static const char* getDomainBySelfEmail(const char* email,char* buffer,int bufferLen);
};
#endif
