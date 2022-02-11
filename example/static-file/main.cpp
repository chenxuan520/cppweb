#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
int main()  
{  
	HttpServer server(5200,true);
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.loadStatic("/file/index.html","index.html");
	server.run(1,3000,"index.html");
    return 0; 
}  

