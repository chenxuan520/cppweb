#include <iostream>
#define CPPWEB_OPENSSL
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
	char ip[30]={0},topUrl[100]={0},endUrl[100]={0};
	string str;
	cout<<"please input the connect web:";
	cin>>str;
	DealHttp::dealUrl(str.c_str(),topUrl,endUrl,100,100);
	cout<<"top:"<<topUrl<<endl;
	cout<<"end:"<<endUrl<<endl;
	ClientTcpIp::getDnsIp(topUrl,ip,30);
	if(strlen(ip)==0)
	{
		printf("get ip wrong\n");
		return -1;
	}
	printf("ip:%s\n",ip);
	unsigned port=5200;
	if(strstr(str.c_str(),"https")!=NULL)
		port=443;
	printf("port:%u\n",port);
	ClientTcpIp client(ip,port);
	cout<<client.lastError();
	DealHttp http;
	DealHttp::Request req;
	req.head.insert(pair<string,string>{"Host",topUrl});
	req.askPath=endUrl;
	req.method="GET";
	req.version="HTTP/1.1";
	char buffer[500]={0},rec[5000]={0};
	http.createAskRequest(req,buffer,500);
	printf("buffer:\n%s\n",buffer);
	if(port!=443)
	{
		if(false==client.tryConnect())
		{
			printf("connect wrong\n");
			return -1;
		}
		if(0>client.sendHost(buffer,strlen(buffer)))
		{
			printf("%d",errno);
			printf("send wrong");
			return -1;
		}
		client.receiveHost(rec,5000);
		printf("%s\n\n",rec);
		return 0;
	}
	if(false==client.tryConnectSSL())
	{
		printf("connect wrong\n");
		return -1;
	}
	if(0>client.sendHostSSL(buffer,strlen(buffer)))
	{
		printf("%d",errno);
		printf("send wrong");
		return -1;
	}
	client.receiveHostSSL(rec,5000);
	printf("%s\n\n",rec);
	return 0;
}
