#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
void func(HttpServer&,DealHttp& http,int)
{
	http.req.analysisRequest(http.info.recText);
	Json json={
		{"ads",http.req.formValue("ads")},
		{"fgf",http.req.formValue("fgf")}
	};
	http.gram.json(DealHttp::STATUSOK,json());
}
void getValue(HttpServer&,DealHttp& http,int)
{
	http.req.analysisRequest(http.info.recText);
	auto value=http.req.routeValue("key");
	std::cout<<"get:"<<http.req.getWildUrl("/get")<<std::endl;
	http.req.urlDecode(value);
	Json json;
	if(value.size()==0)
		json["status"]="find wrong";
	else
	{
		json["status"]="ok";
		json["key"]=value;
	}
	http.gram.json(DealHttp::STATUSOK,json());
}
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	server.get("/root",func);
	server.get("/get*",getValue);
	server.run("./api.html");
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
    return 0; 
}  

