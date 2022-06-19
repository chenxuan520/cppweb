/********************************
	author:chenxuan
	date:2021.7.5
	funtion:this is main cpp for static web
*********************************/
#include "../hpp/cppweb.h"
#include "../hpp/route.h"
#include "../hpp/config.h"
#include "../hpp/argc.h"
using namespace cppweb;
std::string _configFile;
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-17 23:34:19
* description:function of extra config 
***********************************************/
void readExtraSetting(LoadConfig&)
{

}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:from file init server
*********************************/
void readSetting(LoadConfig& load)
{
	load.findConfig("port",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.port=obj.intVal;
					else
					{
						printf("Warning:port not define,use defalt 5200 port\n");
						con.port=5200;
					}
					});
	load.findConfig("memory",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos&&obj.intVal>0)
						con.defaultMemory=obj.intVal;
					else
						con.defaultMemory=1;
					});
	load.findConfig("thread num",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos&&obj.intVal>0)
						con.threadNum=obj.intVal;
					});
	load.findConfig("message print",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.isDebug=obj.boolVal;
					});
	load.findConfig("forward proxy",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.isProxy=obj.boolVal;
					});
	load.findConfig("default file",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.defaultFile=obj.strVal;
					else
						con.defaultFile="";
					});
	load.findConfig("background",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.isBack=obj.boolVal;
					else
						con.isBack=false;
					});
	load.findConfig("log path",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.logPath=obj.strVal;
					else
						con.logPath="access.log";
					});
	load.findConfig("guard",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.isGuard=obj.boolVal;
					else
						con.isGuard=false;
					});
	load.findConfig("auto",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.isAuto=obj.boolVal;
					else
						con.isAuto=true;
					});
	load.findConfig("model",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.model=obj.strVal;
					else
					{
						printf("Warning:model not define,use defalt MULTIPLEXING\n");
						con.model="MULTIPLEXING";
					}
					});
	load.findConfig("long connect",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.isLongConnect=obj.boolVal;
					else
						con.isLongConnect=true;
					});
	load.findConfig("logger",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.isLog=obj.boolVal;
					else
						con.isLog=true;
					});
	load.findConfig("delete path",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						for(auto& now:obj.arr)
							con.deletePath.push_back(now->strVal);
					});
	load.findConfig("replace",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						for(auto& now:obj.arr)
						{
							if((*now)["path"]!=Json::npos&&(*now)["replace"]!=Json::npos)
								con.replacePath.push_back(\
												std::pair<std::string,std::string>{(*now)["path"].strVal,(*now)["replace"].strVal});
							else
								printf("Warning:define replace but format is wrong\n");
						}
					});
	load.findConfig("redirect",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						for(auto& now:obj.arr)
						{
							if((*now)["path"]!=Json::npos&&(*now)["redirect"]!=Json::npos)
								con.redirectPath.push_back(\
												std::pair<std::string,std::string>{(*now)["path"].strVal,(*now)["redirect"].strVal});
							else
								printf("Warning:define redirect but format is wrong\n");
						}
					});
	load.findConfig("reverse proxy",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						for(auto& pnow:obj.arr)
						{
							auto now=*pnow;
							if((now)["weight"]==Json::npos||(now)["host"]==Json::npos||(now)["path"]==Json::npos||(now)["model"]==Json::npos)
							{
								printf("Warning:define proxy but format is wrong\n");
								continue;
							}
							Config::Proxy temp(LoadBalance::TEMPNO);
							auto& strNow=now["model"].strVal;
							if(strNow=="HASH")
								temp.load.addModel(LoadBalance::HASH);
							else if(strNow=="POLLING")
								temp.load.addModel(LoadBalance::POLLING);
							else if(strNow=="POLLRAN")
								temp.load.addModel(LoadBalance::POLLRAN);
							else
								temp.load.addModel(LoadBalance::RANDOM);
							for(unsigned i=0;i<now["host"].arr.size();i++)
							{
								auto arr=now["host"].arr[i]->strVal; 
								temp.host.push_back(arr);
								temp.load.addServer(arr.c_str(),now["weight"].arr[i]->intVal);
							}
							con.proxyMap.insert(std::pair<std::string,Config::Proxy>\
												{now["path"].strVal,temp});
						}
					});
#ifdef CPPWEB_OPENSSL
	load.findConfig("key path",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
					{
						con.keyPath=obj.strVal;
						if(con.keyPath.find(".pem")==con.keyPath.npos)
							printf("Warning:key format must be pem!\n");
					}
					else
						printf("Warning:use ssl but not define key path,https cannot work!\n");
					});
	load.findConfig("cert path",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.certPath=obj.strVal;
					else
						printf("Warning:use ssl but not define cert path,https cannot work!\n");
					});
	load.findConfig("cert password",[](Json::Object& obj,Config& con){
					if(obj!=Json::npos)
						con.passwd=obj.strVal;
					});
#endif
	readExtraSetting(load);
	return;
}
/********************************
	author:chenxuan
	date:2021.7.5
	funtion:server begin
*********************************/
void serverHttp()
{
	if(_config.configFile.size()==0)
		_config.configFile="./config.json";
	FileGet file;
	LoadConfig config(file.getFileBuff(_config.configFile.c_str()));
	if(config.lastError()!=NULL)
	{
		printf("config wrong %s\n",config.lastError());
		exit(0);
	}
	readSetting(config);
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
void dealArgc(ArgcDeal& args)
{
	if(args.getOption("reload")||args.getOption("stop"))
	{
#ifndef _WIN32
		FILE* fp=fopen("./server.pid","r+");
		if(fp==NULL)
			return;
		int pid=0;
		auto flag=fscanf(fp,"%d",&pid);
		if(flag<=0)
			return;
		printf("dealing %d process\n",pid);
		if(pid<=66)
			exit(0);
		kill(pid,2);
		fclose(fp);
		if(args.getOption("stop"))
			exit(0);
		else
		{
			printf("wait for the port unbound...\n");
			sleep(1);
		}
#endif
	}
	if(args.getOption("config"))
	{
		_config.configFile=args.getVari("config");
		printf("read config from %s\n",_config.configFile.c_str());
	}
}
int _main(ArgcDeal& args)
{
	dealArgc(args);
	serverHttp();
	return 0;
}
int main(int argc, char** argv)
{
	ArgcDeal args(argc,argv);
	args.app.name="chenxuanweb server";
	args.app.pfunc=_main;
	args.app.usage="thank using chenxuanweb,if you have any question\n"
		"\tsend email to 1607772321@qq.com to deal problem,thank you!\n"
		"\t! only in linux the argv is accepted\n";
	args.app.version="v2.0";
	args.setOption("reload","restart the server");
	args.setOption("stop","stop the server");
	args.setVari("config","choose the file to be config file");
	return args.run();
}
