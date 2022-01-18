#ifndef _SERVER_H_
#define _SERVER_H_
#include"./http.h"
#include<string.h>
#include<iostream>
#include<winsock2.h>

using namespace std;
namespace cppweb{
class ServerTcpIp{
protected:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now; 
	char* hostip;//host IP 
	char* hostname;//host name
	const char* error;//error hapen
	WSADATA wsa;//apply verson of windows;
	SOCKET sock;//file descriptor of host;
	SOCKET sockC;//file descriptor to sign every client;
	SOCKADDR_IN addr;//IPv4 of host;
	SOCKADDR_IN client;//IPv4 of client;
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
	bool bondhost();//bond myself
	bool setlisten();//set listem to accept
	unsigned int acceptClient();//wait until success
	bool acceptClients(unsigned int* psock);//model two
	inline int receiveOne(void* pget,int len)//model one
	{
		return recv(sockC,(char*)pget,len,0);
	}
	inline int receiveSocket(unsigned int sock,void* prec,int len)//model two
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendClientOne(const void* psend,int len)//model one
	{
		return send(sockC,(char*)psend,len,0);
	}
	inline int sendSocket(unsigned int sock ,const void* psend,int len)//model two
	{
		return send(sock,(char*)psend,len,0);
	}
	void sendEverySocket(const void* psen,int len);//model two
	bool epollModel(void* pget,int len,void* pneed,int (*pfunc)(int,int ,int ,void* ,void*,ServerTcpIp& ));	
	bool selectModel(void* pget,int len,void* pneed,int (*pfunc)(int,int ,int ,void* ,void*,ServerTcpIp& ));
    bool disconnectSocket(SOCKET clisock);
	bool updateSocket(SOCKET* array,int* pcount,int arrayLen);
	bool findSocket(int cliSoc);
	char* getHostIp();
	char* getHostName();
	char* getPeerIp(SOCKET cliSoc,int* pcliPort);
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
		GET,POST,PUT,DELETETO,ALL,
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
	int getCompleteMessage(const void* message,unsigned int messageLen,void* buffer,unsigned int buffLen,int sockCli);
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
	static void loadFile(DealHttp& http,HttpServer& server,int,void* sen,int& len);
	static void sigCliDeal(int pid);
};
class ClientTcpIp{
private:
	WSADATA wsa;//apply for api
	SOCKET sock;//myself
	SOCKADDR_IN addrC;//server information
	char ip[20];//server Ip
	char sen[200];//what you send
	char rec[200];//what you get
	char* hostip;//host ip
	char* hostname;//host name
//	SSL* ssl;
//	SSL_CTX* ctx;
public:
	ClientTcpIp(const char* hostIp,int port=5200);
	~ClientTcpIp();
	void addHostIp(const char* ip,unsigned short port);
	bool tryConnect();
	inline int receiveHost(void* prec,int len)
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendHost(const void* psend,int len)
	{
		return send(sock,(char*)psend,len,0);
	}
	bool disconnectHost();
	char* getSelfIp();
	char* getSelfName();
	static bool getDnsIp(const char* name,char* ip,unsigned int ipMaxLen);
};
}
#endif
