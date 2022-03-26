#include <iostream>  
#include "../../hpp/cppweb.h"

char* passwd=NULL;
#include "./route.h"
using namespace cppweb;
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-26 15:03:52
* description:read password,that's all
***********************************************/
void readPasswd()
{
	passwd=(char*)malloc(sizeof(char)*128);
	if(passwd==NULL)
	{
		printf("init passwd wrong\n");
		exit(0);
	}
	memset(passwd,0,sizeof(char)*128);
	if(false==FileGet::getFileMsg("passwd.txt",passwd,128))
	{
		printf("init passwd wrong\n");
		exit(0);
	}
	if(passwd[strlen(passwd)-1]=='\n')
		passwd[strlen(passwd)-1]=0;
}
int main()
{
	readPasswd();
	/* ProcessCtrl::backGround(); */
	auto flag=chdir("./template");
	if(flag!=0)
	{
		perror("chdir wrong");
		return 0;
	}
	HttpServer server(5200,true);//input the port bound
	server.setMiddleware(middleware);
	server.setLog(LogSystem::recordRequest,LogSystem::recordRequest);
	server.post("/message*",nowPwdFile);
	server.post("/upload*",upload);
	server.post("/mkdir*",mkdirNow);
	server.post("/delete*",mkdirNow);
	server.post("/login",loginIn);
	server.get("/*",sendHtml);
	server.run();
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
    return 0; 
}  

