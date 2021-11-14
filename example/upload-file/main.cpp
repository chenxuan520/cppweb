#include <iostream>  
#include <string.h>
#include "../../lib/server.h"
#include "../../lib/http.h"
using namespace std;
void upload(DealHttp& http,HttpServer& server,int,void* sen,int& len)
{
	char name[100]={0};
	memset(sen,0,sizeof(char)*1024);
	if(0==http.getRecFile(server.recText(),name,100,(char*)sen,1024*1024))
		cout<<"wrong:"<<(char*)server.recText()<<endl;
	else
		FileGet::writeToFile(name,(char*)sen,strlen((char*)sen));
	http.createSendMsg(DealHttp::HTML,(char*)sen,"index.html",&len);
}
int main()  
{  
	HttpServer server(5200,true);
	server.routeHandle(HttpServer::POST,HttpServer::WILD,"/upload",upload);
	server.run(1,5000,"index.html");
    return 0; 
}  

