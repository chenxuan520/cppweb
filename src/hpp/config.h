#pragma once
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-19 11:55:48
* description:this file is for server config,do not change it
***********************************************/
#include "./cppweb.h"
#include "./proxy.h"
using namespace cppweb;
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-17 19:29:17
* description:struct of server config
***********************************************/
struct Config{
	struct Proxy{
		std::vector<std::string> host;
		LoadBalance load;
		Proxy(LoadBalance::Model model=LoadBalance::RANDOM):load(model){};
	};
	struct SSLConfig{
		std::string keyPath;
		std::string certPath;
		std::string passwd;
	};
	bool isLongConnect;
	bool isBack;
	bool isGuard;
	bool isLog;
	bool isAuto;
	bool isDebug;
	bool isProxy;
	int port;
	int defaultMemory;
	int threadNum;
	std::string configFile;
	std::string defaultFile;
	std::string logPath;
	std::string model;
	std::vector<std::string> deletePath;
	std::vector<std::pair<std::string,std::string>> replacePath;
	std::vector<std::pair<std::string,std::string>> redirectPath;
	std::unordered_map<std::string,Proxy> proxyMap;
	SSLConfig sslConfig;
	Config():isLongConnect(true),isBack(false),isGuard(false),isLog(false),isAuto(true),isDebug(true),isProxy(false),port(5200),defaultMemory(1),threadNum(0){};
}_config;
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-17 19:32:20
* description:function of Reverse proxy
***********************************************/
void proxy(HttpServer& server,DealHttp& http,int soc)
{
	auto pathNow=server.getNowRoute();
	char url[128]={0};
	std::string strNow=pathNow->route;
	strNow+='*';
	http.getWildUrl(server.recText(http),pathNow->route,url,128);
	if(_config.proxyMap.find(strNow)!=_config.proxyMap.end())
	{
		auto now=_config.proxyMap[strNow].load;
		char ip[48]={0};
		int port=0,temp=0;
		if(now.getNowModel()!=LoadBalance::HASH)
			strNow=now.getServer();
		else
			strNow=now.getHashServer(ServerTcpIp::getPeerIp(soc,&temp));
		sscanf(strNow.c_str(),"%[^:]:%d",ip,&port);
		memset(server.getSenBuffer(http),0,server.getMaxSenLen(http));
		DealHttp::Request req;
		http.analysisRequest(req,server.recText(http));
		req.askPath=url;
		req.head["Host"]=strNow;
		http.createAskRequest(req,server.getSenBuffer(http),server.getMaxSenLen(http));
		ClientTcpIp client(ip,port);
		if(false==client.tryConnect())
		{
			http.gram.statusCode=DealHttp::STATUSNOFOUND;
			LogSystem::recordRequest(HttpServer::INSIDEERROR,"connect cliwrong",soc);
			return;
		}
		if(false==client.sendHost(server.getSenBuffer(http),strlen((char*)server.getSenBuffer(http))))
		{
			http.gram.statusCode=DealHttp::STATUSNOFOUND;
			LogSystem::recordRequest(HttpServer::INSIDEERROR,"sen cliwrong",soc);
			return;
		}
		memset(server.getSenBuffer(http),0,server.getMaxSenLen(http));
		int socCli=client.getSocket();
		std::string strGet;
		int len=HttpApi::getCompleteHtml(strGet,socCli);
		if(len<=0)
		{
			http.gram.statusCode=DealHttp::STATUSNOFOUND;
			LogSystem::recordRequest(HttpServer::INSIDEERROR,"rec cliwrong",soc);
			return;
		}
		while(len>(int)server.getMaxSenLen(http))
			server.enlagerSenBuffer();
		memcpy(server.getSenBuffer(http),strGet.c_str(),len);
		http.info.staticLen=len;
	}
	else
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
}
// description:deal kill signal
static void _dealSignalKill(int)
{
	LogSystem::recordRequest(HttpServer::SYSLOG,"server stop ",0);
#ifndef _WIN32
	if(ProcessCtrl::childPid!=0)
		kill(ProcessCtrl::childPid,2);
#endif
	exit(0);
}
// description:restart the server
bool _restart=false;
static void _dealSignalRestart(int)
{
	_restart=true;
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-19 11:56:49
* description:main config class
* example: use it by {load.findConfig}
***********************************************/
class LoadConfig{
private:
	Json json;
	Json::Object* rootObj;
	HttpServer* pserver;
	const char* error;
	std::vector<std::pair<std::string,void (*)(Json::Object&,HttpServer&)>> arr;
public:
	LoadConfig(const char* buffer):json(buffer),rootObj(NULL),pserver(NULL),error(NULL)
	{
		if(json.lastError()!=NULL)
		{
			error=json.lastError();
			return;
		}
		rootObj=&json.getRootObj();
		if(*rootObj==Json::npos)
		{
			error=json.lastError();
			return;
		}
	}
	void findConfig(const char* key,void (*pfunc)(Json::Object&,Config&))
	{
		auto root=json.getRootObj();
		pfunc(root[key],_config);
	}
	void findConfig(const char* key,void (*pfunc)(Json::Object&,HttpServer&))
	{
		arr.push_back(std::pair<std::string,void (*)(Json::Object&,HttpServer&)>{key,pfunc});
	}
	void runServer(void (*pfunc)(HttpServer&)=NULL)
	{
		configProcess();
		HttpServer::RunModel model;
		if(_config.model=="THREAD")
			model=HttpServer::THREAD;
		else if(_config.model=="FORK")
			model=HttpServer::FORK;
		else if(_config.model=="REACTOR")
			model=HttpServer::REACTOR;
		else
			model=HttpServer::MULTIPLEXING;
		HttpServer server(_config.port,_config.isDebug,model);
		pserver=&server;
		configServer(server);
		if(pfunc!=NULL)
			pfunc(server);
		if(_config.defaultFile.size()>0)
			server.run(_config.defaultFile.c_str());
		else
			server.run();
		if(server.lastError()!=NULL)
			error=server.lastError();
	}
	inline const char* lastError()
	{
		return error;
	}
private:
	void configProcess()
	{
		if(_config.isBack)
			ProcessCtrl::backGround();
		std::string pid=std::to_string(getpid());
		FileGet::writeToFile("./server.pid",pid.c_str(),pid.size());
		if(_config.isGuard)
			ProcessCtrl::guard();
	}
	void configServer(HttpServer& server)
	{
		server.changeSetting(_config.isDebug,_config.isLongConnect,_config.isAuto,_config.defaultMemory);
		if(_config.isProxy)
		{
			server.all("http://*",ForwardProxy::httpProxy);
			server.routeHandle(HttpServer::CONNECT,"*",ForwardProxy::httpsProxy);
		}
		if(_config.model=="THREAD"&&_config.threadNum!=0)
			server.changeModel(HttpServer::THREAD,_config.threadNum);
		if(_config.model=="REACTOR"&&_config.threadNum!=0)
			server.changeModel(HttpServer::REACTOR,_config.threadNum);
		for(auto& now:_config.deletePath)
			server.deletePath(now.c_str());
		for(auto& now:_config.replacePath)
			server.loadStatic(now.first.c_str(),now.second.c_str());
		for(auto& now:_config.redirectPath)
			server.redirect(now.first.c_str(),now.second.c_str());
		for(auto& now:_config.proxyMap)
			server.all(now.first.c_str(),proxy);
		if(_config.logPath.size()>0)
			LogSystem::defaultName=_config.logPath.c_str();
		if(_config.isLog)
		{
			server.setLog(LogSystem::recordRequest);
#ifndef _WIN32
			signal(SIGINT,_dealSignalKill);
			signal(SIGQUIT,_dealSignalKill);
			signal(SIGTERM,_dealSignalKill);
#endif
		}
		if(rootObj!=NULL)
			for(auto& now:arr)
				now.second((*rootObj)[now.first.c_str()],server);
#ifdef CPPWEB_OPENSSL
		if(FileGet::getFileLen(_config.sslConfig.certPath.c_str())<=0||
		   FileGet::getFileLen(_config.sslConfig.keyPath.c_str())<=0)
		{
			printf("cert file wrong\n");
			exit(0);
		}
		server.loadKeyCert(_config.sslConfig.certPath.c_str(),_config.sslConfig.keyPath.c_str(),_config.sslConfig.passwd.size()==0?NULL:_config.sslConfig.passwd.c_str());
#endif
	}
};
