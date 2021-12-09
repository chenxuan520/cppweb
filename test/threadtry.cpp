#include <iostream>
#include "../hpp/cppweb.h"
using namespace cppweb;
ServerPool server(5200,5);
void* test(void* num)
{
	int soc=(int)(uintptr_t)num,len=0,flag=0;
	char* recbuf=(char*)malloc(sizeof(char)*5000),*sen=(char*)malloc(sizeof(char)*100000);
	memset(recbuf,0,sizeof(char)*5000);
	len=server.receiveSocket(soc,recbuf,sizeof(char)*5000);
	if(len<=0)
	{
		server.threadDeleteSoc(soc);
		return NULL;
	}
	DealHttp http;
	flag=http.autoAnalysisGet(recbuf,sen,100000,"./index.html",&len);
	if(flag==2)
	{
		printf("get %s wrong\n",http.analysisHttpAsk(recbuf));
		return NULL;
	}
	len=server.sendSocket(soc,sen,len);
	if(len<=0)
		perror("send wrong");
	free(recbuf);
	free(sen);
	server.threadDeleteSoc(soc);
	return NULL;
}
int main()
{
	server.bondhost();
	server.setlisten();
	printf("server ok\n");
	while(1)
		server.epollThread(test);
    return 0;
}

