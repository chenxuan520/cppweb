#include <iostream>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
	HttpServer server(5200,true);
	server.setLog(LogSystem::recordRequest,NULL);
	server.get("/stop",[](HttpServer& server,DealHttp&,int){
			   server.stopServer();
			   });
	server.run("index.html");
	return 0;
}

