#define CPPWEB_OPENSSL
#include "../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
	char ip[32]={0};
	ClientTcpIp::getDnsIp("raw.hellogithub.com",ip,32);
	printf("ip:%s\n",ip);
	ClientTcpIp client(ip,443);
	if(false==client.tryConnectSSL())
	{
		perror("connect wrong");
		return 0;
	}
	DealHttp::Request req;
	req.method="GET";
	req.askPath="/hosts";
	req.head["Host"]="raw.hellogithub.com";
	auto buffer=req.createAskRequest();
	if(0>client.sendHost(buffer.c_str(),buffer.size()))
	{
		perror("send");
		return 0;
	}
	string result;
	if(0>client.receiveHost(result))
	{
		perror("recv");
		return 0;
	}
	auto flag=req.analysisRequest(result.c_str());
	if(!flag)
	{
		cout<<"wrong:"<<req.error<<endl;
		return 0;
	}
	printf("body:\n%s\n",result.c_str());
	return 0;
}
