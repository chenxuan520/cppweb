#include"../hpp/sql.h"
#include"../hpp/server.h"
#include"../hpp/http.h"
#include"../hpp/dealask.h"
#include<stdio.h>
#include<string.h>
char indexName[100]="index.html";
int funcTwo(int thing,int num,void* pget,void* sen,ServerTcpIp& server)//main deal func
{
	char ask[200]={0};
	DealHttp http;
	DealAsk dealAsk;
	int len=0;
    int flag=0;
	if(sen==NULL)
		return -1;
	 memset(sen,0,sizeof(char)*10000000);
	// if(false==DealAttack::dealAttack(thing,num,200))
	// {
	// 	DealAttack::attackLog(port,server.getPeerIp(num,&port),"./rec/attackLog.txt");
	// 	server.disconnectSocketEpoll(num);
	// 	return 0;
	// }
	if(thing==0)
		printf("%d is out\n",num);
	if(thing==1)
		printf("%s in %d\n",(char*)pget,num);
	if(thing==2)
	{
		if(true==dealAsk.dealPostAsk(server,http))
			return 0;
		if(false==http.cutLineAsk((char*)pget,"GET"))
			return 0;
		printf("ask:%s\n",(char*)pget);
		printf("http:%s\n",http.analysisHttpAsk(pget));
		strcpy(ask,http.analysisHttpAsk(pget));
        if(false==dealAsk.dealGetAsk(server,http))
        {
			flag=http.autoAnalysisGet((char*)pget,(char*)sen,indexName,&len);
			if(0==flag)
				printf("some thing wrong %s\n",(char*)sen);
			else if(flag==1)
				printf("create auto success\n");
	        else if(flag==2)
	        {
                FILE* fp=fopen("wrong.txt","a+");
                if(fp==NULL)
                    fp=fopen("wrong.txt","w+");
                if(fp==NULL)
                    return 0;
                fprintf(fp,"can not open file %s\n",ask);
                printf("cannot open file %s\n",ask);
                fclose(fp);
            }
		}
		if(false==server.sendSocketAll(num,sen,len))
			printf("send wrong\n");
		else
			printf("send success\n");
	}
	return 0;
}
void chooseModel(unsigned int* port,bool* pflag)
{
	char temp[10]={0};
	printf("please input the import(default 80):");
	scanf("%u",port);
	printf("please input index.html(default index.html):");
	scanf("%s",indexName);
	printf("please input if run in background(default no)y/n:");
	fflush(stdin);
	scanf("%s",temp);
	if(strchr(temp,'y')!=NULL)
		*pflag=true;
	else 
		*pflag=false;
}
void serverHttp()
{
	unsigned int port=80;
	bool is_back=false;
	chooseModel(&port,&is_back);
	if(is_back)
	{
		int pid=0;
		if((pid=fork())!=0)
		{
			printf("pid=%d\n",pid);
			return;
		}
	}
	ServerTcpIp server(port,100);
	int thing=0,num=0;
	char get[2048]={0};
	char* sen=(char*)malloc(sizeof(char)*10000000);
	if(sen==NULL)
		printf("memory wrong\n");
	if(false==server.bondhost())
	{
		printf("bound wrong\n");
		exit(0);
	}
	if(false==server.setlisten())
		exit(0);
	printf("server ip is:%s\nthe server is ok\n",server.getHostIp());
	while(1)
		if(false==server.epollModel(&thing,&num,get,2048,sen,funcTwo))
			break;
	free(sen);
}
int main(int argc, char** argv) 
{
	serverHttp();
	return 0;
}
