#include <iostream>  
#include <string.h>
#include "../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void root(DealHttp & http, HttpServer & server, int, void * send, int & len)
{
	char buffer[100]={0},pwd[100]={0},email[100]={0};
	WebToken token;
	char temp[100]={0},thing[100]={0};
	http.getRouteValue(server.recText(),"password",pwd,100);
	http.getRouteValue(server.recText(),"email",email,100);
	token.createToken("chenxuan",email,temp,100,60);
	sprintf(thing,"http://127.0.0.1:5201/sure/%s",temp);
	Json json;
	json.init(200);
	json.addKeyValue("password",pwd);
	json.addKeyValue("email",email);
	json.addKeyValue("http",thing);
	json.jsonToFile("temp.json");
	http.createSendMsg(DealHttp::JSON,(char*)send,1024*1024,"temp.json",&len);
}
void sure(DealHttp & http, HttpServer & server, int, void * send, int & len)
{
	char buffer[100]={0},email[100]={0};
	http.getWildUrl(server.recText(),"/sure/",buffer,100);
	WebToken token;
	token.decryptToken("chenxuan",buffer,email,100);
	Json json;
	json.init(200);
	json.addKeyValue("email",email);
	json.jsonToFile("temp.json");
	http.createSendMsg(DealHttp::JSON,(char*)send,1024*1024,"temp.json",&len);
}
int main()  
{  
	HttpServer server(5200,true);
	server.changeSetting(true,true,true);
	server.routeHandle(HttpServer::POST,HttpServer::WILD,"/login",root);
	server.routeHandle(HttpServer::GET,HttpServer::WILD,"/sure/",sure);
	server.run(1,5000,"./index.html");
	printf("%s",server.lastError());
    return 0; 
}  

