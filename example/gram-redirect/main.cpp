#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
using namespace std;
int main()  
{  
	/* ProcessCtrl::backGround(); */
	HttpServer server(5200,true);//input the port bound
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.get("/*",[](HttpServer& ser,DealHttp& http,int){
			   unordered_map<string,string> route;
			   http.req.routePairing("/:route",route,(char*)ser.recText());
			   auto result="https://chenxuanweb.top/"+route["route"];
			   http.gram.redirect(result);
			   });
	server.run();
    return 0; 
}  

