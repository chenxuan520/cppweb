#include <iostream>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void pfunc(HttpServer&,DealHttp& http,int)
{
	http.head["hahah"]="en";
	http.info.isContinue=true;
	/* char temp[1000]={0}; */
	/* Json json; */
	/* char* text=json.createObject(200); */
	/* json.addKeyVal(text,Json::STRING,"hah","oko"); */
	/* gram.body=text; */
	/* gram.fileLen=strlen(text); */
	/* gram.typeFile=DealHttp::JSON; */
	/* http.createDatagram(gram,temp,1000); */
	/* server.httpSend(soc,temp,strlen(temp)); */
}
void pfunc1(HttpServer&,DealHttp& http,int){
	http.head["ok"]="lalala";
	http.info.isContinue=true;
}
int main()
{
	HttpServer server(5200,true);
	server.setMiddleware(pfunc);
	server.setMiddleware(pfunc1);
	server.get("/root",[](HttpServer&,DealHttp& http,int){
			   http.gram.body="text";
			   });
	server.run("./index.html");
	return 0;
}
