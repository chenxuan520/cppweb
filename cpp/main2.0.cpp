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
	funtion:from file init server
*********************************/
void readSetting(LoadConfig& load)
{
	load.findConfig("port",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.port=obj->intVal;
					else
						con.port=5200;
					});
	load.findConfig("default file",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.defaultFile=obj->strVal;
					else
						con.defaultFile="";
					});
	load.findConfig("background",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.isBack=obj->boolVal;
					else
						con.isBack=false;
					});
	load.findConfig("guard",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.isGuard=obj->boolVal;
					else
						con.isGuard=false;
					});
	load.findConfig("model",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.model=obj->strVal;
					else
						con.model="";
					});
	load.findConfig("long connect",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.isLongConnect=obj->boolVal;
					else
						con.isLongConnect=true;
					});
	load.findConfig("logger",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.isLog=obj->boolVal;
					else
						con.isLog=true;
					});
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
