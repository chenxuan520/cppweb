#include <iostream>
#include "../../hpp/cppweb.h"
using namespace cppweb;
using namespace std;
void pfuncTwo(HttpServer&,DealHttp&,int,DealHttp::Datagram& gram)
{
	char temp[]="is a text file";
	gram.typeFile=DealHttp::TXT;
	gram.body=temp;
	std::cout<<gram.body<<endl;
	gram.fileLen=strlen(temp);
	gram.statusCode=DealHttp::STATUSOK;
}
void pfunc(HttpServer& server,DealHttp& http,int,DealHttp::Datagram& gram)
{
	char url[100]={0};
	Json json;
	char* strJson=json.createObject(200);
	http.getWildUrl(server.recText(),"/root/",url,100);//get url wild
	/* http.getRouteValue(url,"name",value,30);//get name value */
	/* printf("value:%s\n",value); */
	json.addKeyVal(strJson,Json::STRING,"name",url);
	json.addKeyVal(strJson,Json::STRING,"welcome","you");
	gram.typeFile=DealHttp::JSON;
	gram.body=strJson;
	gram.statusCode=DealHttp::STATUSOK;
	gram.fileLen=strlen(strJson);
}
void pfuncThree(HttpServer& server,DealHttp& http,int,DealHttp::Datagram&)
{
	char* sen=(char*)server.getSenBuff();
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
	server.routeHandle(HttpServer::GET,HttpServer::WILD,"/root/",pfunc);
	server.get(HttpServer::ONEWAY,"/txt",pfuncTwo);
	server.get(HttpServer::WILD,"/try/",pfuncThree);
	server.run(1,4000,"index.html");
    return 0; 
}  
