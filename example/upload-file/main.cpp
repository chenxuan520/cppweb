#include <iostream>  
#include <string.h>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void upload(HttpServer& server,DealHttp& http,int,DealHttp::Datagram& gram)
{
    char name[100]={0};
	void* sen=server.getSenBuff();
    printf("%s\n",(char*)server.recText());
	int flen=server.getRecLen();
    if(flen>1024*1024)
    {
		gram.statusCode=DealHttp::STATUSNOFOUND;
		return;
    }
    flen=http.getRecFile(server.recText(),server.getRecLen(),name,100,(char*)sen,1024*1024);
	printf("%s\n%d\n",sen,flen);
    FileGet::writeToFile(name,(char*)sen,flen);
	gram.typeFile=DealHttp::JSON;
	gram.body="{\"status\":\"ok\"}";
}

int main()  
{  
	HttpServer server(5200,true);
	server.routeHandle(HttpServer::POST,HttpServer::WILD,"/upload",upload);
	server.run("index.html");
    return 0; 
}  

