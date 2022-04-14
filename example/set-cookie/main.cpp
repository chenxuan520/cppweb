#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
void cookie(HttpServer& server,DealHttp& http,int)
{
	auto buf=http.getCookie(server.recText(),"key");
	std::unordered_map<std::string,std::string> temp;
	http.req.routePairing((char*)server.recText(),"/cookie/:cookie",temp);
	if(buf.size()==0)
	{
		http.gram.body="ready to setting cookie";
		if(temp.find("cookie")==temp.end())
			http.gram.cookie["key"]=http.designCookie("cookie",10);
		else
			http.gram.cookie["key"]=http.designCookie(temp["cookie"].c_str(),10);
		return;
	}
	Json json={
		{"key",buf},
		{"status","ok"}
	};
	http.gram.body=json();
	http.gram.typeFile=DealHttp::JSON;
}
int main()  
{  
	HttpServer server(5200,true);
	server.get("/cookie/*",cookie);
	server.run("index.html");
    return 0; 
}  

