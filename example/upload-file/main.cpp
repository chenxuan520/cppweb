#include <iostream>  
#include <string.h>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void upload(HttpServer& server,DealHttp& http,int soc,DealHttp::Datagram& gram)
{
    char name[100]={0};
	void* sen=server.getSenBuff();
	DealHttp::Request req;
	http.analysisRequest(req,server.recText());
	if(req.head.find("Content-Length")==req.head.end())
	{
		gram.body="message wrong";
		return;
	}
	int flen=0;
	sscanf(req.head["Content-Length"].c_str(),"%d",&flen);
	printf("conlen:%d\n",flen);
	void *temp=malloc(sizeof(char)*(flen+1000));
	if(temp==NULL)
	{
		gram.statusCode=DealHttp::STATUSNOFOUND;
		return;
	}
	unsigned len=server.getCompleteMessage(server.recText(),server.getRecLen(),temp,sizeof(char)*(flen+1000),soc);
	printf("len %d\n",len);
    flen=http.getRecFile(temp,len,name,100,(char*)sen,1024*1024*3);
    FileGet::writeToFile(name,(char*)sen,flen);
	gram.typeFile=DealHttp::JSON;
	gram.body="{\"status\":\"ok\"}";free(temp);
}

int main()  
{  
	HttpServer server(5200,true);
	server.changeSetting(true,true,false,3);
	server.routeHandle(HttpServer::POST,HttpServer::WILD,"/upload",upload);
	server.run("index.html");
    return 0; 
}  

