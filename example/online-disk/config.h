#pragma once
#include "../../hpp/cppweb.h"
using namespace cppweb;
struct _Config{
	std::string passwd;
	std::string storePath;
	std::string logPath;
	bool isLog;
	int port;
	int tokenTime;
	_Config():passwd("123456"),storePath("./template"),\
			  isLog(false),port(5200),tokenTime(120){};
}_config;
bool configServer(const char* configPath)
{
	if(configPath==NULL)
		return false;
	auto strGet=FileGet::getFileString(configPath);
	Json json(strGet.c_str());
	if(json["log"]!=NULL)
		_config.isLog=json["log"]->boolVal;
	if(json["port"]!=NULL)
		_config.port=json["port"]->intVal;
	if(json["passwd"]!=NULL)
		_config.passwd=json["passwd"]->strVal;
	if(json["log path"]!=NULL)
		_config.logPath=json["log path"]->strVal;
	if(json["store"]!=NULL)
		_config.storePath=json["store"]->strVal;
	if(json["token time"]!=NULL)
		_config.tokenTime=json["token time"]->intVal;
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
