#include "../hpp/argc.h"
using namespace cppweb;
int _main(ArgcDeal& app){
	auto value=app.getVari("config");
	printf("get %s\n",value.c_str());
	return 0;
}
int main(int argc,char** argv){
	ArgcDeal msg(argc,argv);
	msg.app.name="argc test";
	msg.app.usage="a no use test help";
	msg.app.version="v1.0";
	msg.app.pfunc=_main;
	msg.setVari("config","the config file",true);
	msg.setOption("p","the test option",true);
	return msg.run();
}
