#include <iostream>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
	HttpServer server(5201,true);
	server.loadKeyCert("./cacert.pem","./privkey.pem");
	server.run("./index.html");
	printf("error:%s\n",server.lastError());
	return 0;
}
