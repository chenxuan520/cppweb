#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
int main()  
{  
	HttpServer server(5200,true);
	server.run(1,3000,"index.html");
    return 0; 
}  

