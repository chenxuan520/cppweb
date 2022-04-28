#include <iostream>  
#include <string.h>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void upload(HttpServer& server,DealHttp& http,int soc)
{
    char name[100]={0};
	DealHttp::Request req;
	http.analysisRequest(req,server.recText());
	if(req.head.find("Content-Length")==req.head.end())
	{
		http.gram.body="message wrong";
		return;
	}
	int flen=0;
	sscanf(req.head["Content-Length"].c_str(),"%d",&flen);
	printf("conlen:%d\n",flen);
	void *temp=malloc(sizeof(char)*(flen+1000));
	if(temp==NULL)
	{
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
		return;
	}
	memset(temp,0,sizeof(char)*(flen+1000));
	unsigned len=server.getCompleteMessage(soc);
	printf("len %d\n",len);
    flen=http.getRecFile(server.recText(),len,name,100,(char*)temp,sizeof(char)*(flen+1000));
    FileGet::writeToFile(name,(char*)temp,flen);
	http.gram.typeFile=DealHttp::JSON;
	http.gram.body="{\"status\":\"ok\"}";
	free(temp);
}

int main()  
{  
	HttpServer server(5200,true);
	server.changeSetting(true,true,true,3);
	server.routeHandle(HttpServer::POST,"/upload",upload);
	server.run("index.html");
    return 0; 
}  

