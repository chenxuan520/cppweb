#include <iostream>  
#include "../../hpp/cppweb.h"
#include "./route.h"
using namespace cppweb;
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	server.post("/message*",nowPwdFile);
	server.post("/upload*",upload);
	server.post("/mkdir*",mkdirNow);
	server.post("/delete*",mkdirNow);
	server.get("/*",sendHtml);
	server.run();
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
    return 0; 
}  

