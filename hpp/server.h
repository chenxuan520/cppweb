#ifndef _SERVER_H_
#define _SERVER_H_
#include"./http.h"
#include<string.h>
#include<iostream>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<unistd.h>
#include<netdb.h>

using namespace std;
/********************************
	author:chenxuan
	date:2021/11/11
	funtion:the server class
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
	bool selectModel(void* pget,int len,void* pneed,int (*pfunc)(Thing ,int ,int,void* ,void*,ServerTcpIp& ));
	bool updateSocket(int* array,int* pcount);//get epoll array
	bool findSocket(int cliSoc);//find if socket is connect
	char* getHostName();//get self name
	char* getHostIp();//get self ip
	char* getPeerIp(int cliSoc,int* pcliPort);//get ip and port by socket
	bool epollModel(void* pget,int len,void* pneed,int (*pfunc)(Thing,int ,int ,void* ,void*,ServerTcpIp& ));
	bool disconnectSocket(int clisock);//disconnect from socket
	inline const char* getLastError()//get last error
	{
		return error;
	}
};
class HttpServer:private ServerTcpIp{
public:
	enum RouteType{
		ONEWAY,WILD,STATIC,
	};
	enum AskType{
		GET,POST,ALL,
	};
	struct RouteFuntion{
		AskType ask;
		RouteType type;
		char route[100];
		const char* path;
		void (*pfunc)(DealHttp&,HttpServer&,int num,void* sen,int&);
	};
private:
	RouteFuntion* array;
	RouteFuntion* pnowRoute;
	void* getText;
	unsigned int max;
	unsigned int now;
	int textLen;
	bool isDebug;
	bool isLongCon;
	bool isFork;
	void (*clientIn)(HttpServer&,int num,void* ip,int port);
	void (*clientOut)(HttpServer&,int num,void* ip,int port);
public:
	HttpServer(unsigned port,bool debug=false);
	~HttpServer();
	bool clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port));
	bool clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port));
	bool routeHandle(AskType ask,RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
	bool loadStatic(const char* route,const char* staticPath);
	bool get(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
	bool post(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
	bool all(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
	void run(unsigned int memory,unsigned int recBufLenChar,const char* defaultFile);
	int httpSend(int num,void* buffer,int sendLen);
	int httpRecv(int num,void* buffer,int bufferLen);
	void changeSetting(bool debug,bool isLongCon,bool isForkModel);
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
	inline RouteFuntion* getNowRoute()
	{
		return pnowRoute;
	}
	int func(int num,void* pget,void* sen,unsigned int senLen,const char* defaultFile,HttpServer& server);
	void epollHttp(void* pget,int len,unsigned int senLen,void* pneed,const char* defaultFile);
	void forkHttp(void* pget,int len,unsigned int senLen,void* pneed,const char* defaultFile);
	static void loadFile(DealHttp& http,HttpServer& server,int senLen,void* sen,int& len);
	static void sigCliDeal(int pid);
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
//	SSL* ssl;
//	SSL_CTX* ctx;
public:
	ClientTcpIp(const char* hostIp,unsigned short port);
	~ClientTcpIp();
	void addHostIp(const char* ip,unsigned short port=0);
	bool tryConnect();
	inline int receiveHost(void* prec,int len)
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendHost(const void* ps,int len)
	{
		return send(sock,(char*)ps,len,0);
	}
	bool disconnectHost();
	char* getSelfIp();
	char* getSelfName(char* hostname,unsigned int bufferLen);
	static bool getDnsIp(const char* name,char* ip,unsigned int ipMaxLen);
//	bool sslInit()
//	{
//		const SSL_METHOD* meth=SSLv23_client_method();
//		if(meth==NULL)
//			return false;
//		ctx=SSL_CTX_new(meth);
//		if(ctx==NULL)
//			return false;
//		ssl=SSL_new(ctx);
//		if(NULL==ssl)
//			return false;
//		SSL_set_fd(ssl,sock);
//		int ret=SSL_connect(ssl);
//		if(ret==-1)
//			return false;
//		return true;
//	}
//	inline int sendhostSSL(const void* psen,int len)
//	{
//		return SSL_write(ssl,psen,len);
//	}
//	inline int receiveHostSSL(void* buffer,int len)
//	{
//		return SSL_read(ssl,buffer,len);
//	}
};
#endif
