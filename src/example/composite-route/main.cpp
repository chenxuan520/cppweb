#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
struct Temp{
	int soc;
	char cookie[128];
};
void login(HttpServer&,DealHttp& http,int soc){
	auto flag=http.req.getCookie("key",http.info.recText);
	if(flag.size()>0){
		Temp temp={soc,{0}};
		strncpy(temp.cookie,flag.c_str(),128);
		http.setVar<Temp>("cookie",temp);
		http.setVar<int>("id",soc);
		http.info.isContinue=true;
	}else{
		http.gram.json(DealHttp::STATUSOK,Json::createJson({{"status","find wrong"}}));
	}
}
void message(HttpServer&,DealHttp& http,int){
	Temp* pstr=(Temp*)http.getVar("cookie");
	int* pid=(int*)http.getVar("id");
	if(pstr==NULL){
		http.gram.json(DealHttp::STATUSNOFOUND,Json::createJson({{"status","wrong"}}));
	}else{
		http.gram.json(DealHttp::STATUSOK,Json::createJson({{"cookie",pstr->cookie},{"soc",pstr->soc},{"id",*pid}}));
	}
}
void setLogin(HttpServer&,DealHttp& http,int){
	auto flag=http.req.getCookie("key",http.info.recText);
	if(flag.size()>0){
		http.gram.txt(DealHttp::STATUSOK,"ok");
		return;
	}
	cppweb::KeyMap key;
	http.req.routePairing("/cookie/:id",key,(char*)http.info.recText);
	http.gram.cookie["key"]=http.designCookie(key["id"].c_str(),10,"/");
	http.gram.json(DealHttp::STATUSOK,Json::createJson({{"status","ok"},{"id",key["id"]}}));
}
int main()  
{  
	HttpServer server(5200,true);//input the port bound
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.get("/",{login,message});
	server.get("/cookie/*",setLogin);
	server.run();
    return 0; 
}  

