#include <iostream>  
#include "./lib/server.h"
#include "./lib/http.h"
using namespace std;
void root(DealHttp & http, HttpServer & server, int, void * send, int & len)
{
	char buffer[100]={0};
	http.getAskRoute(server.recText(),"/root/",buffer,100);
	cout<<buffer<<endl;
	http.getWildUrl(server.recText(),"/root/",buffer,100);
	cout<<buffer<<endl;
	Json json;
	json.jsonInit(100);
	json.addKeyValue("okp","op");
	json.endJson();
	json.jsonToFile("temp.json");
	http.createSendMsg(DealHttp::JSON,(char*)send,"temp.json",&len);
}
int main()  
{  
	HttpServer server(5201);
	server.routeHandle(HttpServer::GET,HttpServer::WILD,"/root/",root);
	server.run(1,"./index.html");
    return 0; 
}  

