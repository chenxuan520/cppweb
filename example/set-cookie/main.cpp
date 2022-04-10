#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
void cookie(HttpServer& server,DealHttp& http,int)
{
	auto buf=http.getCookie(server.recText(),"key");
	if(buf.size()==0)
	{
		http.gram.body="ready to setting cookie";
		http.gram.cookie["key"]=http.designCookie("cookie ok",10);
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
	server.get("/cookie",cookie);
	server.run("index.html");
    return 0; 
}  

