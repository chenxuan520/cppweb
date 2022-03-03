#include <iostream>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void login(DealHttp & http, HttpServer & , int , void * sen, int & len)
{
	DealHttp::Datagram gram;
	gram.statusCode=DealHttp::STATUSOK;
	Json json;
	char* result=json.createObject(200);
	json.addKeyVal(result,Json::STRING,"root","hahah");
	json.addKeyVal(result,Json::EMPTY,"null");
	gram.typeFile=DealHttp::JSON;
	gram.body=result;
	gram.fileLen=strlen(result);
	len=http.createDatagram(gram,sen,1024*1024);
	len=strlen((char*)sen);
	printf("sen:%s\n%d\n",(char*)sen,len);
}
int main()
{
	HttpServer server(5200,true);
	server.get(HttpServer::ONEWAY,"/login",login);
	server.run(1,3000,"index.html");
	printf("%s\n",server.lastError());
}

