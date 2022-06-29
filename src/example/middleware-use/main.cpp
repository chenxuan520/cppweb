#include <iostream>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void pfunc(HttpServer& server,DealHttp& http,int soc)
{
	http.head["hahah"]="en";
	server.continueNext(soc,http);
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
int main()
{
	HttpServer server(5200,true);
	server.setMiddleware(pfunc);
	server.get("/root",[](HttpServer&,DealHttp& http,int){
			   http.gram.body="text";
			   });
	server.run("./index.html");
	return 0;
}
