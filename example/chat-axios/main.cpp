#include <iostream>  
#include <string.h>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void root(HttpServer&,DealHttp& http,int)
{
	Json json{
		{"status","ok"}
	};
	http.gram.head["Access-Control-Allow-Origin"]="*";
	http.gram.body=json();
}
int main()  
{  
	HttpServer server(5200,true);
	server.all("/root",root);
	server.get("/stop",[](HttpServer& server,DealHttp&,int){
				server.stopServer();
				});
	server.run("b.html");
	printf("%s",server.lastError());
    return 0; 
}  

