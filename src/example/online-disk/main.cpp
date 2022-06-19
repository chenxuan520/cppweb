#include <iostream>  
#include "../../hpp/cppweb.h"
#include "./config.h"
#include "./route.h"
using namespace cppweb;
int main(int argc,char** argv)
{
	ArgcDeal arg(argc,argv);
	arg.app.name="online-disk";
	arg.app.usage="store file online and easily to get web link";
	arg.app.version="v1.1";
	arg.app.pfunc=_main;
	arg.setOption("d","run app in background");
	arg.setOption("g","restart if the app fall auto");
	return arg.run();
}
