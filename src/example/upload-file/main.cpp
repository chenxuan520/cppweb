#include <iostream>  
#include <string.h>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void upload(HttpServer& server,DealHttp& http,int soc)
{
	char name[100]={0};
	DealHttp::Request req;
	if(false==http.analysisRequest(req,server.recText(http)))
	{
		Json json={{"status","analysisHttpAsk wrong"}};
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	if(req.head.find("Content-Length")==req.head.end())
	{
		http.gram.body="message wrong";
		return;
	}
	char path[128]={0};
	http.getWildUrl(server.recText(http),server.getNowRoute()->route,path,sizeof(char)*128);
	req.askPath=".";
	req.askPath+=path;
	int flen=0;
	sscanf(req.head["Content-Length"].c_str(),"%d",&flen);
	void *temp=malloc(sizeof(char)*(flen+1000));
	if(temp==NULL)
	{
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
		return;
	}
	memset(temp,0,sizeof(char)*(flen+1000));
	flen=http.getRecFile(server.recText(http),server.getRecLen(http),name,100,(char*)temp,sizeof(char)*(flen+1000));
	req.askPath+=name;
	cout<<"file:"<<req.askPath<<endl;
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

