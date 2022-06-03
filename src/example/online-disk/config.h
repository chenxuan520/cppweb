#pragma once
#include "../../hpp/cppweb.h"
using namespace cppweb;
struct _Config{
	std::string passwd;
	std::string storePath;
	std::string logPath;
	bool isLog;
	bool isVerify;
	int port;
	int tokenTime;
	_Config():passwd("123456"),storePath("./template"),\
			  isLog(false),isVerify(true),port(5200),tokenTime(120){};
}_config;
bool configServer(const char* configPath)
{
	if(configPath==NULL)
		return false;
	auto strGet=FileGet::getFileString(configPath);
	Json json(strGet.c_str());
	auto root=json.getRootObj();
	if(root["log"]!=Json::npos)
		_config.isLog=root["log"].boolVal;
	if(root["port"]!=Json::npos)
		_config.port=root["port"].intVal;
	if(root["passwd"]!=Json::npos)
		_config.passwd=root["passwd"].strVal;
	if(root["log path"]!=Json::npos)
		_config.logPath=root["log path"].strVal;
	if(root["store"]!=Json::npos)
		_config.storePath=root["store"].strVal;
	if(root["token time"]!=Json::npos)
		_config.tokenTime=root["token time"].intVal;
	if(root["verify"]!=Json::npos)
		_config.isVerify=root["verify"].boolVal;
	if(_config.storePath.size()!=0)
	{
		auto flag=chdir(_config.storePath.c_str());
		if(flag!=0)
		{
			perror("chdir wrong");
			exit(0);
		}
	}
	if(_config.logPath.size()!=0&&_config.isLog)
		LogSystem::defaultName=_config.logPath.c_str();
	return true;
}
void dealArgv(char** argv,int argc)
{
	if(argc<=1)
		return;
	else if(strcmp(argv[1],"-d")==0)
	{
		ProcessCtrl::backGround();
		return;
	}
	else if(strcmp(argv[1],"-dg")==0)
	{
		ProcessCtrl::backGround();
		ProcessCtrl::guard();
		return;
	}
	else
		printf("welcome to online disk,\n -d\t run in background\n -gt run in back with guard");
}
