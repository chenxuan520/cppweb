#include <iostream>  
#include <string.h>
#include "../lib/server.h"
#include "../lib/http.h"
using namespace std;
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
	json.init(100);
	json.addKeyValue("password",pwd);
	json.addKeyValue("email",email);
	json.addKeyValue("http",thing);
	json.jsonToFile("temp.json");
	http.createSendMsg(DealHttp::JSON,(char*)send,"temp.json",&len);
}
void sure(DealHttp & http, HttpServer & server, int, void * send, int & len)
{
	char buffer[100]={0},email[100]={0};
	http.getWildUrl(server.recText(),"/sure/",buffer,100);
	WebToken token;
	token.decryptToken("chenxuan",buffer,email,100);
	Json json;
	json.jsonInit(200);
	json.addKeyValue("email",email);
	json.endJson();
	json.jsonToFile("temp.json");
	http.createSendMsg(DealHttp::JSON,(char*)send,"temp.json",&len);
}
int main()  
{  
	HttpServer server(5201,true);
	server.routeHandle(HttpServer::POST,HttpServer::WILD,"/login",root);
	server.routeHandle(HttpServer::GET,HttpServer::WILD,"/sure/",sure);
	server.run(1,"./index.html");
    return 0; 
}  

