#include "./cppweb.h"
using namespace cppweb;
struct Config{
	struct Proxy{
		std::string host;
		int port;
		std::string proxyPath;
		Proxy():port(5200){};
	};
	bool isLongConnect;
	bool isBack;
	bool isGuard;
	bool isLog;
	bool isAuto;
	bool isDebug;
	int port;
	std::string defaultFile;
	std::string model;
	std::vector<std::string> deletePath;
	std::vector<std::string,std::string> replacePath;
	std::unordered_map<std::string,Proxy> proxyMap;
	Config():isLongConnect(true),isBack(false),isGuard(false),isLog(false),isAuto(true),port(5200){};
}config;
void proxy(HttpServer& server,DealHttp& http,int soc)
{
	auto pathNow=server.getNowRoute();
	if(config.proxyMap.find(pathNow->route)!=config.proxyMap.end())
	{
		auto now=config.proxyMap[pathNow->route];
		char ip[48]={0};
		ClientTcpIp::getDnsIp(now.host.c_str(),ip,48);
		ClientTcpIp client(ip,now.port);
		if(false==client.tryConnect())
		{
			http.gram.statusCode=DealHttp::STATUSNOFOUND;
			LogSystem::recordRequest("connect cliwrong",soc);
			return;
		}
		if(false==client.sendHost(server.recText(),server.getRecLen()))
		{
			http.gram.statusCode=DealHttp::STATUSNOFOUND;
			LogSystem::recordRequest("sen cliwrong",soc);
			return;
		}
		server.selfCreate(client.receiveHost(server.getSenBuffer(),server.getMaxSenLen()));
	}
	else
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
}
class LoadConfig{
private:
	Json json;
	const char* error;
public:
	LoadConfig(const char* buffer):json(buffer),error(NULL)
	{
		if(json.lastError()!=NULL)
		{
			error=json.lastError();
			return;
		}
	}
	void findConfig(const char* key,void (*pfunc)(Json::Object*,Config&))
	{
		auto root=*json.getRootObj();
		pfunc(root[key],config);
	}
	void runServer(void (*pfunc)(HttpServer&)=NULL)
	{
		configProcess();
		HttpServer::RunModel model;
		if(config.model=="THREAD")
			model=HttpServer::THREAD;
		else if(config.model=="FORK")
			model=HttpServer::FORK;
		else
			model=HttpServer::MULTIPLEXING;
		HttpServer server(config.isDebug,config.port,model);
		configServer(server);
		if(pfunc!=NULL)
			pfunc(server);
		if(config.defaultFile.size()>0)
			server.run(config.defaultFile.c_str());
		else
			server.run();
	}
private:
	void configProcess()
	{
		if(config.isBack)
			ProcessCtrl::backGround();
		if(config.isGuard)
			ProcessCtrl::guard();
	}
	void configServer(HttpServer& server)
	{
		server.changeSetting(true,config.isLongConnect,config.isAuto);
		for(auto& now:config.deletePath)
			server.deletePath(now.c_str());
		for(auto& now:config.replacePath)
			server.loadStatic(now.first.c_str(),now.second.c_str());
		for(auto& now:config.proxyMap)
			server.all(now.first.c_str(),proxy);
		if(config.isLog)
			server.setLog(LogSystem::recordRequest,LogSystem::recordRequest);
	}
	inline const char* lastError()
	{
		return error;
	}
};
