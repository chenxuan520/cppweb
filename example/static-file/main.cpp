#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
int main()  
{  
	HttpServer server(5200,true);
	server.loadStatic("/root/*","./temp/");
	server.loadStatic("/file","index.html");
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.run("index.html");
    return 0; 
}  

