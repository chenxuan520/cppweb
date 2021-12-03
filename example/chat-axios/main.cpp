#include <iostream>  
#include <string.h>
#include "../../lib/server.h"
#include "../../lib/http.h"
using namespace std;
void root(DealHttp& http,HttpServer& ,int,void* sen,int& len)
{
	Json json;
	json.init(200);
	json.addKeyValue("status","ok");
	len=strlen((char*)sen);
	http.customizeAddTop(sen,1024*1024,200,strlen(json.resultText()));
	http.customizeAddHead(sen,1024*1024,"Access-Control-Allow-Origin","*");
	len=http.customizeAddBody(sen,1024*1024,json.resultText(),strlen(json.resultText()));
}
int main()  
{  
	HttpServer server(5200,true);
	server.routeHandle(HttpServer::ALL,HttpServer::WILD,"/root",root);
	server.run(1,6000,"index.html");
    return 0; 
}  

