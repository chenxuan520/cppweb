#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
void cookie(HttpServer& server,DealHttp& http,int)
{
	http.req.analysisRequest(server.recText());
	auto buf=http.req.getCookie("key");
	printf("%s\n",(const char*)server.recText());
	if(http.req.head.find("Cookie")==http.req.head.end())
		printf("fing wrong\n");
	else
		printf("cookie:%s\n\n",http.req.head["Cookie"].c_str());
	std::unordered_map<std::string,std::string> temp;
	http.req.routePairing("/cookie/:cookie/:time",temp,(char*)server.recText());
	if(buf.size()==0)
	{
		int live=5;
		http.gram.body="ready to setting cookie";
		if(temp["time"].size()!=0)
		{
			sscanf(temp["time"].c_str(),"%d",&live);
			printf("live:%d\n",live);
		}
		else
			printf("ok\n");
		if(live<=0)
			live=5;
		printf("live:%d\n",live);
		if(temp["cookie"].size()==0)
			http.gram.cookie["key"]=http.designCookie("cookie",live);
		else
			http.gram.cookie["key"]=http.designCookie(temp["cookie"].c_str(),live);
		return;
	}
	Json json={
		{"key",buf},
		{"status","ok"}
	};
	http.gram.json(DealHttp::STATUSOK,json());
}
int main()  
{  
	HttpServer server(5200,true);
	server.get("/cookie/*",cookie);
	server.run("index.html");
    return 0; 
}  

