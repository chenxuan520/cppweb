/********************************
	author:chenxuan
	date:2021.7.5
	funtion:this is main cpp for static web
*********************************/
//#include"../hpp/sql.h"
#include"../hpp/server.h"
#include"../hpp/http.h"
#include"../hpp/route.h"
#include<stdio.h>
#include<string.h>
char indexName[100]="index.html";
int memory=0;
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
    printf("please input memory(M):");
    fflush(stdin);
    scanf("%d",&memory);
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
	fp=fopen("my.ini","w+");
	if(fp==NULL)
		return;
	fprintf(fp,"%u %s %d %s",*port,indexName,memory,temp);
	fclose(fp);
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:from file init server
*********************************/
void ifChoose(bool* pb,unsigned int* pport,bool* is_back)
{
	char temp[10]={0};
    FILE* fp=fopen("my.ini","r+");
    if(fp==NULL)
    {
        *pb=false;
        return;
    }
    if(fscanf(fp,"%u",pport)!=1)
    {
        *pb=false;
        printf("port=%u\n",*pport);
        return;
    }
    if(fscanf(fp,"%s",indexName)!=1)
    {
        *pb=false;
        printf("name:%s\n",indexName);
        return;
    }
    if(fscanf(fp,"%d",&memory)!=1)
    {
        *pb=false;
        printf("memory=%d\n",memory);
        return;
    }
    if(fscanf(fp,"%s",temp)!=1)
    {
		*pb=false;
		printf("is back wrong\n");
		return;
	}
	if(strchr(temp,'y')!=NULL)
		*is_back=true;
	else
		*is_back=false;
    *pb=true;
    fclose(fp);
    return;
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:from argv to init server
*********************************/
bool ifArgc(int argc,char** argv,bool* pis_back,unsigned int* pport)
{
	if(argc!=5)
		return false;
	if(sscanf(argv[1],"%d",pport)!=1)
    {
        printf("init wrong\n");
        return false;
    }
    sscanf(argv[2],"%s",indexName);
    if(sscanf(argv[3],"%d",&memory)!=1)
    {
        printf("memory wrong\n");
        return false;
    }
    if(strchr(argv[4],'y')!=NULL)
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
		#ifndef _WIN32
		if((pid=fork())!=0)
		{
			printf("pid=%d\n",pid);
			return;
		}
		#endif
	}
	HttpServer server(port,true);
	addHandle(server);
	server.run(memory,4000,"./index.html");
}
int main(int argc, char** argv) 
{
	serverHttp(argc,argv);
	return 0;
}
