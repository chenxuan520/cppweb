#include <iostream>
#include "../../hpp/cppweb.h"
using namespace cppweb;
using namespace std;
void pfuncTwo(HttpServer&,DealHttp& http,int)
{
	char temp[]="is a text file";
	http.gram.typeFile=DealHttp::TXT;
	http.gram.body=temp;
	std::cout<<http.gram.body<<endl;
	http.gram.fileLen=strlen(temp);
	http.gram.statusCode=DealHttp::STATUSOK;
}
void pfunc(HttpServer& server,DealHttp& http,int)
{
	char url[100]={0};
	http.getWildUrl(server.recText(),"/root/",url,100);//get url wild
	Json json={{"name",url},{"welcome","you"}};
	http.gram.json(DealHttp::STATUSOK,json());
}
void pfuncThree(HttpServer& server,DealHttp& http,int)
{
	DealHttp::Request req;
	unordered_map<string,string> tree;
	req.routePairing((char*)server.recText(),"/try/:id/:name",tree);
	Json json={
		{"id",tree["id"]},
		{"name",tree["name"]}
	};
	char* sen=(char*)server.getSenBuffer();
	http.customizeAddTop(sen,1024*1024,200,strlen(json()));
	int len=http.customizeAddBody(sen,1024*1024,json(),strlen(json()));
	server.selfCreate(len);
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
