#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
int main()  
{  
	/* ProcessCtrl::backGround(); */
	HttpServer server(5200,true);//input the port bound
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.get("/*",[](HttpServer&,DealHttp& http,int){
			   http.gram.redirect("https://chenxuanweb.top");
			   });
	server.run();
    return 0; 
}  

