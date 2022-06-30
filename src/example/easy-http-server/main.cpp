#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
int main()  
{  
	HttpServer server(5200,false,cppweb::HttpServer::REACTOR);//input the port bound
	server.changeSetting(false,false);//close long connect and do not print message
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.run("./index.html");
    return 0; 
}  

