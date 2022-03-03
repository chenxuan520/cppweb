#include <iostream>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void func(ServerPool& server,int soc)
{
	char rec[10000]={0};
	char* sen=(char*)malloc(sizeof(char)*1000000);
	if(sen==NULL)
	{
		printf("malloc worng\n");
		return;
	}
	server.receiveSocket(soc,rec,10000);
	DealHttp http;
	printf("get %s\n",http.analysisHttpAsk(rec));
	int len=0;
	int flag=http.autoAnalysisGet(rec,sen,1000000,"./index.html",&len);
	if(flag==2)
		printf("open wrong\n");
	if(len<=0)
	{
		printf(" len wrong\n%s\n",rec);
		free(sen);
		return ;
	}
	if(server.sendSocket(soc,sen,len)<=0)
		printf("%s\n",server.lastError());
	else
		printf("send success\n");
	server.disconnectSocket(soc);
	free(sen);
	printf("end thread\n");
}
int main()
{
	ServerPool server(5200,10);
	if(false==server.bondhost())
	{
		printf("bound wrong %s\n",server.lastError());
		return 0;
	}
	server.setlisten();
	server.threadModel(func);
	return 0;
}

