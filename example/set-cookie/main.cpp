#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
void cookie(HttpServer& server,DealHttp& http,int)
{
	char buffer[100]={0};
	http.getCookie(server.recText(),"key",buffer,100);
	if(strlen(buffer)==0)
	{
		http.gram.body="ready to setting cookie";
		http.gram.cookie["key"]=http.designCookie("cookie ok",10);
		return;
	}
	Json json={
		{"key",(const char*)buffer},
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

