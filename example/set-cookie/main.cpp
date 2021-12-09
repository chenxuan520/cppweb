#include <iostream>  
#include "../../lib/server.h"
#include "../../lib/http.h"
using namespace cppweb;
void cookie(DealHttp & http, HttpServer & server, int , void * sen, int & len)
{
	char buffer[100]={0};
	Json json;
	json.init(200);
	http.getKeyValue(server.recText(),"key",buffer,100);
	json.addKeyValue("key",buffer);
	http.customizeAddTop(sen,1000000,DealHttp::STATUSOK,strlen(json.resultText()));
	if(NULL==http.getCookie(server.recText(),"key",buffer,100))
	{
		http.setCookie(sen,1000000,"key","wew",10);
	}
	len=http.customizeAddBody(sen,1000000,json.resultText(),strlen(json.resultText()));
}
int main()  
{  
	HttpServer server(5200);
	server.get(HttpServer::ONEWAY,"/cookie",cookie);
	server.run(1,3000,"index.html");
    return 0; 
}  

