#include <iostream>
#include "../../lib/server.h"
#include "../../lib/http.h"
using namespace cppweb;
using namespace std;
void pfunc(DealHttp & http, HttpServer & server, int , void * sen, int & len)
{
	char url[100]={0},value[30]={0};
	Json json;
	char* strJson=json.createObject(200);
	http.getWildUrl(server.recText(),"/root/",url,100);//get url wild
	http.getRouteValue(url,"name",value,30);//get name value
	printf("value:%s\n",value);
	json.addKeyVal(strJson,Json::STRING,"name",url);
	json.addKeyVal(strJson,Json::STRING,"welcome","you");
	DealHttp::Datagram gram;
	gram.typeFile=DealHttp::JSON;
	gram.body=strJson;
	gram.statusCode=DealHttp::STATUSOK;
	gram.fileLen=strlen(strJson);
	len=http.createDatagram(gram,sen,1024*1024);
}
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.routeHandle(HttpServer::GET,HttpServer::WILD,"/root/",pfunc);
	server.run(1,4000,"index.html");
    return 0; 
}  
