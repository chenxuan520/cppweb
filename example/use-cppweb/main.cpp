#include <iostream>  
#include "../../hpp/cppweb.h"
int main()  
{  
	HttpServer server(5200);
	server.run(1,3000,"index.html");
    std::cout << "Hello world" << std::endl; 
    return 0; 
}  

