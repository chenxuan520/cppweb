#include <iostream>
#define CPPWEB_OPENSSL
#include "../../hpp/cppweb.h"
#include "config.h"
using namespace std;
using namespace cppweb;
int main()
{
	Config config("config.json");
	if(config.error!=NULL){
		printf("config wrong %s\n",config.error);
		return 0;
	}
	auto buffer=config.req.createAskRequest();
	if(config.req.error!=NULL){
		printf("req create wrong\n");
		return 0;
	}
	cout<<"buffer"<<endl<<buffer<<endl;
	ClientTcpIp cli(config.host.ip.c_str(),config.host.port);
	if(config.host.port!=443){
		if(!cli.tryConnect()){
			printf("connect wrong\n");
			return 0;
		}
	}else{
		if(!cli.tryConnectSSL()){
			printf("connect wrong\n");
			return 0;
		}
	}
	int num=0;
	if(config.host.port!=443){
		auto soc=cli.getSocket();
		num=SocketApi::sendSocket(soc,buffer.c_str(),buffer.size());
	}else{
		auto ssl=cli.getSSL();
		num=SocketApi::sendSocket(ssl,buffer.c_str(),buffer.size());
	}
	if(num<=0){
		printf("send wrong");
		return 0;
	}
	if(config.host.port!=443){
		auto soc=cli.getSocket();
		buffer.clear();
		num=HttpApi::getCompleteHtml(buffer,soc);
	}else{
		auto ssl=cli.getSSL();
		buffer.clear();
		num=HttpApi::getCompleteHtmlSSL(buffer,ssl);
	}
	if(num<=0){
		printf("recv wrong");
		return 0;
	}
	cout<<"buffer"<<endl<<buffer<<endl;
	return 0;
}
