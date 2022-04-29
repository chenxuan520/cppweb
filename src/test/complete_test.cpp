#include "../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
	char ip[32]={0};
	ClientTcpIp::getDnsIp("www.androidftp.top",ip,32);
	ClientTcpIp cli("47.106.146.155",5203);
	if(false==cli.tryConnect())
	{
		perror("connect");
		return 0;
	}
	auto sock=cli.getSocket();
	DealHttp::Request req;
	req.method="GET";
	req.askPath="/";
	req.head["Host"]="127.0.0.1";
	auto result=req.createAskRequest();
	cli.sendHost(result.c_str(),result.size());
	string buffer;
	HttpApi::getCompleteHtml(buffer,sock);
	/* SocketApi::receiveSocket(sock,buffer); */
	/* SocketApi::recvSockBorder(sock,buffer,"\r\n\r\n",0); */
	/* auto flag=req.analysisRequest(buffer.c_str()); */
	/* if(!flag) */
	/* { */
	/* 	printf("error:%s\n",req.error); */
	/* 	return -1; */
	/* } */
	/* int len=0; */
	/* if(req.head.find("Content-Length")==req.head.end()) */
	/* { */
	/* 	printf(" head not found\n"); */
	/* 	cout<<buffer<<endl; */
	/* 	return 0; */
	/* } */
	/* else */
	/* 	sscanf(req.head["Content-Length"].c_str(),"%d",&len); */
	/* SocketApi::recvSockSize(sock,buffer,len); */
	cout<<buffer<<endl;
	return 0;
}
