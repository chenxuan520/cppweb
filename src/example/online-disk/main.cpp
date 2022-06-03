#include <iostream>  
#include "../../hpp/cppweb.h"
#include "./config.h"
#include "./route.h"
using namespace cppweb;
int main()
{
	/* ProcessCtrl::backGround(); */
	/* ProcessCtrl::guard(); */
	if(false==configServer("./config.json"))
	{
		printf("find config.json wrong\n");
		return 0;
	}
	HttpServer server(_config.port,true);//input the port bound
	if(_config.isLog)
		server.setLog(LogSystem::recordRequest,LogSystem::recordRequest);
	if(_config.isVerify)
		server.setMiddleware(middleware);
	server.post("/message*",nowPwdFile);
	server.post("/upload*",upload);
	server.post("/mkdir*",mkdirNow);
	server.post("/delete*",mkdirNow);
	server.post("/move*",moveFile);
	server.post("/login",loginIn);
	server.post("/logout",loginOut);
	server.post("/link*",apiLink);
	server.get("/*",sendHtml);
	auto group=server.createGroup("/edit");
	{
		group.post("/save",saveEdit);
		group.post("/get",getEdit);
	}
	server.run();
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
    return 0; 
}  

