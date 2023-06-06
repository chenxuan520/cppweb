#define CPPWEB_OPENSSL
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void dict(const char* str)
{
	char ip[32]={0};
	auto serverIP=ClientTcpIp::getDnsIp("api.interpreter.caiyunai.com");
	if(serverIP=="")
	{
		printf("dns wrong\n");
		return ;
	}
	ClientTcpIp client(serverIP,443);
	auto flag=client.tryConnectSSL();
	if(!flag)
	{
		printf("connect %s wrong\n",ip);
		return;
	}
	DealHttp::Request req;
	Json json={
		{"trans_type","en2zh"},
		{"source",str}
	};
	req.method="POST";
	req.askPath="/v1/dict";
	req.head["Host"]="api.interpreter.caiyunai.com";
	req.head["Content-Type"]="application/json;charset=utf-8";
	req.head["X-Authorization"]="token:qgemv4jr1y38jyq6vhvi";
	req.head["Connection"]="keep-alive";
	req.body=json();
	auto buffer=req.createAskRequest();
	if(0>=client.sendHost(buffer.c_str(),buffer.size()))
	{
		printf("send wrong\n");
		return ;
	}
	buffer.clear();
	if(0>=HttpApi::getCompleteHtmlSSL(buffer,client.getSSL()))
	{
		printf("recv wrong\n");
		return;
	}
	req.analysisRequest(buffer.c_str(),buffer.size());
	auto body=req.hexStrToUtf8(req.body);
	flag=json.analyseText(body.c_str());
	if(flag==false)
	{
		printf("json error:%s\n",json.lastError());
		return;
	}
	auto root=json.getRootObj();
	printf("English:\n%s\nChinese:\n",str);
	if(root["dictionary"]["explanations"]!=Json::npos)
	{
		auto now=root["dictionary"]["explanations"];
		for(auto strNow:now.arr)
			printf("%s\n",strNow->strVal.c_str());
	}
	else
		printf("message not found\n");
	return;
}
int main(int argc,char** argv)
{
	if(argc!=2)
	{
		printf("argv wrong,please input the url to argv\n");
		exit(0);
	}
	dict(argv[1]);
	return 0;
}
