#include <iostream>  
#include "../../lib/server.h"
#include "../../lib/http.h"
using namespace cppweb;
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.run(1,4000,"./index.html");
    return 0; 
}  

