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
	funtion:config server by self
*********************************/
void selfConfig(const Config&,HttpServer&)
{

}
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
	load.findConfig("memory",[](Json::Object* obj,Config& con){
					if(obj!=NULL&&obj->intVal>0)
						con.defaultMemory=obj->intVal;
					else
						con.defaultMemory=1;
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
	load.findConfig("auto",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.isAuto=obj->boolVal;
					else
						con.isAuto=true;
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
	load.findConfig("delete path",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						for(auto& now:obj->arr)
							con.deletePath.push_back(now->strVal);
					});
	load.findConfig("replace",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						for(auto& now:obj->arr)
						{
							if((*now)["path"]!=NULL&&(*now)["replace"]!=NULL)
								con.replacePath.push_back(\
												std::pair<std::string,std::string>{(*now)["path"]->strVal,(*now)["replace"]->strVal});
						}
					});
	load.findConfig("redirect",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						for(auto& now:obj->arr)
						{
							if((*now)["path"]!=NULL&&(*now)["redirect"]!=NULL)
								con.redirectPath.push_back(\
												std::pair<std::string,std::string>{(*now)["path"]->strVal,(*now)["redirect"]->strVal});
						}
					});
	load.findConfig("reverse proxy",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						for(auto& pnow:obj->arr)
						{
							auto now=*pnow;
							if((now)["weight"]==NULL||(now)["host"]==NULL||(now)["path"]==NULL||(now)["model"]==NULL)
								return;
							Config::Proxy temp(LoadBalance::TEMPNO);
							auto& strNow=now["model"]->strVal;
							if(strNow=="HASH")
								temp.load.addModel(LoadBalance::HASH);
							else if(strNow=="POLLING")
								temp.load.addModel(LoadBalance::POLLING);
							else if(strNow=="POLLRAN")
								temp.load.addModel(LoadBalance::POLLRAN);
							else
								temp.load.addModel(LoadBalance::RANDOM);
							for(unsigned i=0;i<now["host"]->arr.size();i++)
							{
								auto arr=now["host"]->arr[i]->strVal; 
								temp.host.push_back(arr);
								temp.load.addServer(arr.c_str(),now["weight"]->arr[i]->intVal);
							}
							con.proxyMap.insert(std::pair<std::string,Config::Proxy>\
												{now["path"]->strVal,temp});
						}
					});
#ifdef CPPWEB_OPENSSL
	load.findConfig("key path",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.keyPath=obj->strVal;
					});
	load.findConfig("cert path",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.certPath=obj->strVal;
					});
	load.findConfig("cert password",[](Json::Object* obj,Config& con){
					if(obj!=NULL)
						con.passwd=obj->strVal;
					});
#endif
	load.configToServer(selfConfig);
	return;
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:server begin
*********************************/
void serverHttp()
{
	FileGet file;
	LoadConfig config(file.getFileBuff("./config.json"));
	readSetting(config);
	if(config.lastError()!=NULL)
	{
		printf("config wrong %s\n",config.lastError());
		exit(0);
	}
	config.runServer(addHandle);
	if(config.lastError()!=NULL)
	{
		printf("config wrong %s\n",config.lastError());
		exit(0);
	}
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:reload the server
*********************************/
void dealArgc(int argc,char** argv)
{
	if(argc==1)
		return;
	if(strcmp(argv[1],"reload")==0||strcmp(argv[1],"--reload")==0||\
	   strcmp(argv[1],"stop")==0||strcmp(argv[1],"--stop")==0)
	{
#ifndef _WIN32
		FILE* fp=fopen("./server.pid","r+");
		if(fp==NULL)
			return;
		int pid=0;
		fscanf(fp,"%d",&pid);
		printf("dealing %d process\n",pid);
		if(pid<=66)
			exit(0);
		kill(pid,2);
		fclose(fp);
		if(strcmp(argv[1],"stop")==0||strcmp(argv[1],"--stop")==0)
			exit(0);
#endif
	}
	else
	{
		printf("thank using chenxuanweb,if you have any question\n"
			   "send email to 1607772321@qq.com to deal problem\nTHANK YOU!\n"
			   "--help\t\tget the help\n"
			   "--stop\t\tstop the server\n"
			   "--reload\t\treload the server\n"
			   "! only in linux the argv is accepted\n");
		exit(0);
	}
}
int main(int argc, char** argv) 
{
	dealArgc(argc,argv);
	serverHttp();
	return 0;
}
