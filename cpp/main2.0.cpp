/********************************
	author:chenxuan
	date:2021.7.5
	funtion:this is main cpp for static web
*********************************/
#include "../hpp/cppweb.h"
#include "../hpp/route.h"
#include<stdio.h>
#include<string.h>
using namespace cppweb;
char indexName[100]="index.html";
bool isGuard=false;
bool isLongConnect=true;
bool isFork=false;
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:if no argc an file use it to init
*********************************/
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
	FILE* fp=fopen("my.ini","r+");
	if(fp!=NULL)
	{
		fclose(fp);
		return;
	}
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:from file init server
*********************************/
void ifChoose(bool* pb,unsigned int* pport,bool* is_back)
{
	char temp[4000]={0};
	FileGet file;
	file.getFileMsg("config.json",temp,4000);
	Json json(temp);
	if(json["port"]!=NULL)
		*pport=json["port"]->intVal;
	else
		*pb=false;
	if(json["default file"]!=NULL)
		strcpy(indexName,json["default file"]->strVal.c_str());
	else
		*pb=false;
	if(json["background"]!=NULL)
		*is_back=json["background"]->boolVal;
	else
		*pb=false;
	if(json["guard"]!=NULL)
		isGuard=json["guard"]->boolVal;
	else
		*pb=false;
	if(json["fork"]!=NULL)
		isFork=json["fork"]->boolVal;
	else
		*pb=false;
	if(json["long connect"]!=NULL)
		isLongConnect=json["long connect"]->boolVal;
	else
		*pb=false;
	*pb=true;
	return;
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:from argv to init server
*********************************/
bool ifArgc(int argc,char** argv,bool* pis_back,unsigned int* pport)
{
	if(argc!=4)
		return false;
	if(sscanf(argv[1],"%d",pport)!=1)
	{
		printf("init wrong\n");
		return false;
	}
	sscanf(argv[2],"%s",indexName);
	if(strstr(argv[3],"true")!=NULL)
		*pis_back=true;
	return true;
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:server begin
*********************************/
void serverHttp(int argc,char** argv)
{
	unsigned int port=80;
	bool is_back=false,is_choose=false;
	ifChoose(&is_choose,&port,&is_back);
	if(ifArgc(argc,argv,&is_back,&port)==false&&is_choose==false)
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
	if(isGuard)
		Guard guard;
	HttpServer server(port,true);
	addHandle(server);
	server.run(indexName);
}
int main(int argc, char** argv) 
{
	serverHttp(argc,argv);
	return 0;
}
