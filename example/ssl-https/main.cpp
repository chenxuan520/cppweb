#include <iostream>
#define CPPWEB_OPENSSL
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
	HttpServer server(5201,true,HttpServer::THREAD,5);
	server.loadKeyCert("./cacert.pem","./privkey.pem","123456");
	server.run("./index.html");
	printf("error:%s\n",server.lastError());
	return 0;
}
