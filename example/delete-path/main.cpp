#include <iostream>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
	HttpServer server(5200,true);
	server.deletePath("test");
	server.run("./index.html");
	if(NULL!=server.lastError())
	{
		printf("%s\n",server.lastError());
		return -1;
	}
	return 0;
}

