#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
void func(HttpServer& server,DealHttp& http,int)
{
	http.req.analysisRequest(server.recText());
	printf("old:%s\n",(char*)server.recText());
	printf("new:%s %s %s\n",http.req.method.c_str(),http.req.askPath.c_str(),http.req.version.c_str());
	for(auto iter=http.req.head.begin();iter!=http.req.head.end();iter++)
		printf("%s:%s\n",iter->first.c_str(),iter->second.c_str());
	printf("body:%s\n",http.req.body);
	Json json={
		{"ads",http.req.formValue(server.recText(),"ads")},
		{"fgf",http.req.formValue(server.recText(),"fgf")}
	};
	http.gram.json(DealHttp::STATUSOK,json());
}
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	server.all("/root",func);
	server.run("./api.html");
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
    return 0; 
}  

