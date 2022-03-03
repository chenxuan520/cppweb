#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
void func(HttpServer& server,DealHttp& http,int)
{
	DealHttp::Request req;
	http.analysisRequest(req,server.recText());
	printf("old:%s\n",(char*)server.recText());
	printf("new:%s %s %s\n",req.method.c_str(),req.askPath.c_str(),req.version.c_str());
	for(auto iter=req.head.begin();iter!=req.head.end();iter++)
		printf("%s:%s\n",iter->first.c_str(),iter->second.c_str());
	printf("body:%s\n",req.body);
	http.gram.statusCode=DealHttp::STATUSOK;
	http.gram.typeFile=DealHttp::JSON;
	http.gram.body="{\"ha\":\"ha\"}";
}
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	server.all("/root",func);
	server.run("./index.html");
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
    return 0; 
}  

