#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
void func(DealHttp& http,HttpServer& server,int,void* sen,int& len)
{
	DealHttp::Request req;
	http.analysisRequest(req,server.recText());
	printf("old:%s\n",(char*)server.recText());
	printf("new:%s %s %s\n",req.method.c_str(),req.askPath.c_str(),req.version.c_str());
	for(auto iter=req.head.begin();iter!=req.head.end();iter++)
		printf("%s:%s\n",iter->first.c_str(),iter->second.c_str());
	printf("body:%s\n",req.body);

	DealHttp::Datagram gram;
	gram.statusCode=DealHttp::STATUSOK;
	gram.typeFile=DealHttp::JSON;
	gram.body="{\"ha\":\"ha\"}";
	gram.fileLen=11;
	len=http.createDatagram(gram,sen,1024*1024);
}
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	server.all(HttpServer::ONEWAY,"/root",func);
	server.run(1,4000,"./index.html");
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
    return 0; 
}  

