#include <iostream>
#define CPPWEB_OPENSSL
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void wget(const char* str)
{
	char ip[32]={0},topUrl[128]={0},endUrl[128]={0};
	unsigned port=5200;
	bool isHttps=false;
	DealHttp::dealUrl(str,topUrl,endUrl,128,128);
	if(strstr(str,"127.0.0.1")!=NULL)
	{
		sscanf(topUrl,"%[^:]:%u",ip,&port);
		if(strstr(str,"https")!=NULL)
			isHttps=true;
	}
	else
	{
		ClientTcpIp::getDnsIp(topUrl,ip,32);
		if(strstr(str,"https")!=NULL)
		{
			isHttps=true;
			port=443;
		}
		else
			port=80;
	}
	if(strlen(ip)==0)
	{
		printf("dns ip wrong\n");
		exit(0);
	}
	printf("ip:%s\n",ip);
	printf("port:%u\n",port);
	ClientTcpIp client(ip,port);
	if(client.lastError()!=NULL)
	{
		printf("error%s\n;",client.lastError());
		exit(0);
	}
	DealHttp http;
	DealHttp::Request req;
	req.head.insert(pair<string,string>{"Host",topUrl});
	req.askPath=endUrl;
	req.method="GET";
	req.version="HTTP/1.1";
	char buffer[500]={0},rec[5000]={0};
	http.createAskRequest(req,buffer,500);
	printf("buffer:\n%s\n",buffer);
	if(!isHttps)
	{
		if(false==client.tryConnect())
		{
			printf("connect wrong\n");
			exit(0);
		}
		if(0>client.sendHost(buffer,strlen(buffer)))
		{
			printf("%d",errno);
			printf("send wrong");
			exit(0);
		}
		client.receiveHost(rec,5000);
		printf("%s\n\n",rec);
	}
	else
	{
		if(false==client.tryConnectSSL())
		{
			printf("connect wrong\n");
			exit(0);
		}
		if(0>client.sendHost(buffer,strlen(buffer)))
		{
			printf("%d",errno);
			printf("send wrong");
			exit(0);
		}
		client.receiveHost(rec,5000);
		printf("%s\n\n",rec);
	}
}
int main(int argc,char** argv)
{
	if(argc!=2)
	{
		printf("argv wrong,please input the url to argv\n");
		exit(0);
	}
	wget(argv[1]);
	return 0;
}
