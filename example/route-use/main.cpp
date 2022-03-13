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
	Json json;
	char* strJson=json.createObject();
	http.getWildUrl(server.recText(),"/root/",url,100);//get url wild
	/* http.getRouteValue(url,"name",value,30);//get name value */
	/* printf("value:%s\n",value); */
	json.addKeyVal(strJson,Json::STRING,"name",url);
	json.addKeyVal(strJson,Json::STRING,"welcome","you");
	http.gram.typeFile=DealHttp::JSON;
	http.gram.body=strJson;
	http.gram.statusCode=DealHttp::STATUSOK;
	http.gram.fileLen=strlen(strJson);
}
void pfuncThree(HttpServer& server,DealHttp& http,int)
{
	char* sen=(char*)server.getSenBuffer();
	http.customizeAddTop(sen,1024*1024,200,strlen("{\"as\":1}"));
	int len=http.customizeAddBody(sen,1024*1024,"{\"as\":1}",strlen("{\"as\":1}"));
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
