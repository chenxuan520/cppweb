#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
int main()  
{  
	HttpServer server(5200,true);
	server.changeSetting(false,false,false);
	server.run("index.html");
	return 0; 
}  

