#include <iostream>
#include "../../lib/server.h"
#include "../../lib/http.h"
void pfunc(DealHttp & http, HttpServer & server, int , void * sen, int & len)
{
	char url[100]={0},value[30]={0};
	Json json;
	json.init(100);//json init
	http.getWildUrl(server.recText(),"/root/",url,100);//get url wild
	http.getRouteValue(url,"name",value,30);//get name value
	json.addKeyValue("name",value);
	json.addKeyValue("welcome","you");
	json.endJson();
	json.jsonToFile("temp");
	http.createSendMsg(DealHttp::JSON,(char*)sen,"temp",&len);
}
int main()  
{  
	HttpServer server(5200);//input the port bound
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.routeHandle(HttpServer::GET,HttpServer::WILD,"/root/",pfunc);
	server.run(1,4000,"index.html");
    return 0; 
}  
