/********************************
	author:chenxuan
	date:2021.7.5
	funtion:this is main cpp for static web
*********************************/
#include "../hpp/cppweb.h"
#include "../hpp/route.h"
#include "../hpp/config.h"
#include<stdio.h>
#include<string.h>
using namespace cppweb;
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
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:from file init server
*********************************/
void ifChoose()
{
	FileGet file;
	Json json(file.getFileBuff("config.json"));
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
	HttpServer server(port,true);
	addHandle(server);
	server.run(indexName);
}
int main(int argc, char** argv) 
{
	serverHttp(argc,argv);
	return 0;
}
