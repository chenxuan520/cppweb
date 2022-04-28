#include <iostream>  
#include "../../hpp/cppweb.h"
using namespace cppweb;
char* buffer=NULL;
int func(ServerTcpIp::Thing thing,int soc,ServerTcpIp& server,void*)
{
	if(thing==ServerTcpIp::CPPIN)
	{
		printf("%d in\n",soc);
		return 0;
	}
	memset(buffer,0,1000000);
	int len=server.receiveSocket(soc,buffer,1000000);
	if(len<=0)
		return soc;
	DealHttp http;
	int flag=http.autoAnalysisGet(buffer,buffer,1000000,"./index.html",&len);
	if(flag==2||flag==0)
		printf("ask:%s\n",buffer);
	server.sendSocket(soc,buffer,len);
	return 0;
}
int main()  
{
	ServerTcpIp server(5200);
	buffer=(char*)malloc(sizeof(char)*1000000);
	if(buffer==NULL)
	{
		printf("buffer wrong\n");
		return -1;
	}
	if(false==server.bondhost())
	{
		printf("bound wrong\n");
		return -1;
	}
	if(false==server.setlisten())
	{
		printf("%s\n",server.lastError());
		return -1;
	}
	while(1)
		server.selectModel(func,NULL);
    return 0; 
}  

