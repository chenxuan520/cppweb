#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
struct Temp{
	int soc;
	char cookie[128];
};
struct Temp1{
	int soc;
	std::string cookie;
};
void login(HttpServer&,DealHttp& http,int soc){
	auto flag=http.req.getCookie("key",http.info.recText);
	if(flag.size()>0){
		Temp temp={soc,{0}};
		Temp1 temp1={soc,flag};
		strncpy(temp.cookie,flag.c_str(),128);
		http.setVar<Temp>("cookie",temp);
		http.setVar<Temp1>("cookie1",temp1,true);
		http.setVar<int>("id",soc);
		http.info.isContinue=true;
	}else{
		http.gram.json(DealHttp::STATUSOK,Json::createJson({{"status","find wrong"}}));
	}
}
void message(HttpServer&,DealHttp& http,int){
	Temp* pstr=(Temp*)http.getVar("cookie");
	Temp1* pstr1=(Temp1*)http.getVar("cookie1");
	if(pstr==NULL||pstr1==NULL){
		http.gram.json(http.STATUSOK,"{\"status\",\"wrong\"}");
		return;
	}
	int* pid=(int*)http.getVar("id");
	if(pstr==NULL){
		http.gram.json(DealHttp::STATUSNOFOUND,Json::createJson({{"status","wrong"}}));
	}else{
		http.gram.json(DealHttp::STATUSOK,Json::createJson({
														   {"cookie",pstr->cookie},{"soc",pstr->soc},{"id",*pid},
														   {"cookie1",pstr1->cookie},{"soc1",pstr1->soc},
														   }));
	}
	http.deleteVar<Temp1>("cookie1");
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

