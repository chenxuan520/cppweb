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
	int port;
	std::vector<std::string> deletePath;
	std::unordered_map<std::string,std::string> replacePath;
	std::vector<Proxy> proxyArr;
	Config():isLongConnect(true),isBack(false),isGuard(false),isLog(false),isAuto(true),port(5200){};
};
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
	void findConfig(const char* key,void (*pfunc)(Json::Object*,void*),void* argv)
	{
		auto root=*json.getRootObj();
		pfunc(root[key],argv);
	}
	void configServer(HttpServer& server,const Config& config)
	{
		if(config.isBack)
			ProcessCtrl::backGround();
		if(config.isGuard)
			ProcessCtrl::guard();
		server.changeSetting(true,config.isLongConnect,config.isAuto);
	}
	inline const char* lastError()
	{
		return error;
	}
};
