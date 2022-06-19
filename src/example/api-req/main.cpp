#include <iostream>
#define CPPWEB_OPENSSL
#include "../../hpp/cppweb.h"
#include "../../hpp/argc.h"
#include "config.h"
using namespace std;
using namespace cppweb;
int _main(ArgcDeal& app)
{
	auto fileName=app.getVari("config");
	if(fileName.size()==0){
		fileName="./config.json";
	}
	Config config(fileName);
	if(config.error!=NULL){
		printf("config wrong %s\n",config.error);
		return 0;
	}
	auto buffer=config.req.createAskRequest();
	if(config.req.error!=NULL){
		printf("req create wrong\n");
		return 0;
	}
	if(app.getOption("v")){
		cout<<"request:"<<endl<<buffer<<endl;
	}
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
	if(app.getOption("b")){
		string temp=buffer.substr(buffer.find("\r\n\r\n")+4);
		cout<<temp<<endl;
	}else if(app.getOption("t")){
		string temp=buffer.substr(0,buffer.find("\r\n\r\n"));
		cout<<temp<<endl;
	}else{
		cout<<"gram"<<endl<<buffer<<endl;
	}
	return 0;
}
int main(int argc,char** argv){
	ArgcDeal arg(argc,argv);
	arg.app.name="api-test-tool";
	arg.app.usage="a useful tool to create http ask and get gram";
	arg.app.version="v1.0";
	arg.app.pfunc=_main;
	arg.setVari("config","change the config file,default is config.json");
	arg.setOption("t","only show the gram head");
	arg.setOption("b","only show the body");
	arg.setOption("v","show request text");
	return arg.run();
}
