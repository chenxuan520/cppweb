#include <iostream>
#include "../../hpp/cppweb.h"
using namespace cppweb;
using namespace std;
void pfuncTwo(HttpServer&,DealHttp& http,int)
{
	char temp[]="is a text file";
	http.gram.statusCode=DealHttp::STATUSOK;
	http.gram.txt(DealHttp::STATUSOK,temp);
}
void pfunc(HttpServer&,DealHttp& http,int)
{
	auto url=http.req.getWildUrl("/root/");//get url wild
	Json json={{"name",url},{"welcome","you"}};
	http.gram.json(DealHttp::STATUSOK,json());
}
void pfuncThree(HttpServer&,DealHttp& http,int)
{
	http.req.analysisRequest(http.info.recText);
	unordered_map<string,string> tree;
	http.req.routePairing("/try/:id/:name",tree);
	Json json={
		{"id",tree["id"]},
		{"name",tree["name"]}
	};
	char* sen=(char*)http.info.sendBuffer->buffer;
	http.customizeAddTop(sen,http.info.sendBuffer->getMaxSize(),200,strlen(json()));
	int len=http.customizeAddBody(sen,http.info.sendBuffer->getMaxSize(),json(),strlen(json()));
	http.info.staticLen=len;
}
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.routeHandle(HttpServer::GET,"/root/*",pfunc);
	server.get("/lam*",[](HttpServer&,DealHttp& http,int)->void{
		http.gram.body="hahaha";
	});
	server.get("/txt",pfuncTwo);
	server.get("/try/*",pfuncThree);
	server.run("index.html");
    return 0; 
}  
