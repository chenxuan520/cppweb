#include <iostream>  
#include <string.h>
#include "../../lib/server.h"
#include "../../lib/http.h"
using namespace std;
void upload(DealHttp& http,HttpServer& server,int num,void* sen,int& len)
{
    char name[100]={0},buffer[6000];
    memset(sen,0,sizeof(char)*1024*1024);
    printf("%s\n",(char*)server.recText());
    int flen=server.getCompleteMessage(server.recText(),server.recLen(),sen,1024*1024,num);
    printf("%s\n",(char*)sen);
    if(flen>6000)
    {
            http.createSendMsg(DealHttp::NOFOUND,(char*)sen,6000,NULL,&len);
            return;
    }
    flen=http.getRecFile(sen,name,100,buffer,6000);
    FileGet::writeToFile(name,buffer,flen);
    http.createSendMsg(DealHttp::HTML,(char*)sen,1024*1024,"index.html",&len);
}

int main()  
{  
	HttpServer server(5200,true);
	server.routeHandle(HttpServer::POST,HttpServer::WILD,"/upload",upload);
	server.run(1,6000,"index.html");
    return 0; 
}  

