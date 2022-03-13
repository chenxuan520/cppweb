#pragma once
#include "./cppweb.h"
using namespace cppweb;
class LoadBalance{
public:
	enum Model{
		POLLING,//one by one 
		POLLRAN,//polling by random
		RANDOM,//random find
		HASH,//use hash
		TEMPNO//a temp model do not use it
	};
private:
	Model model;
	std::vector<std::string> ipAddr;
	std::vector<int> ipWeight;
	unsigned int pollingNum;
	unsigned int widghtAll;
public:
	LoadBalance(Model choice)
	{
		this->model=choice;
		pollingNum=0;
		widghtAll=0;
		char* ptemp=(char*)malloc(0);
		srand((unsigned int)((long long int)ptemp));
		free(ptemp);
	}
	bool addModel(Model choice)
	{
		if(model!=TEMPNO)
			return false;
		model=choice;
		return true;
	}
	void addServer(const char* ip,int weight)
	{
		switch(model)
		{
			case HASH:
			case POLLING:
			case POLLRAN:
				for(int i=0;i<weight;i++)
					ipAddr.push_back(ip);
				break;
			case RANDOM:
				ipWeight.push_back(weight);
				ipAddr.push_back(ip);
				widghtAll+=weight;
				break;
			default:
				break;
		}
	}
	const char* getServer()
	{
		const char* temp=NULL;
		int num=0;
		if(ipAddr.size()==0)
			return NULL;
		switch(model)
		{
			case POLLING:
				if(pollingNum>=ipAddr.size())
					pollingNum=0;
				temp=ipAddr[pollingNum++].c_str();
				break;
			case POLLRAN:
				temp=ipAddr[rand()%ipAddr.size()].c_str();
				break;
			case RANDOM:
				num=rand()%widghtAll;
				for(unsigned int i=0;i<ipAddr.size();i++)
					if(ipWeight[i]>num)
					{
						temp=ipAddr[i].c_str();
						break;
					}
					else
						num-=ipWeight[i];
				break;
			case HASH:
			case TEMPNO:
				break;
		}
		return temp;
	}
	bool deleteServer(const char* ip)
	{
		bool flag=false;
		std::vector<std::string>::iterator temp=ipAddr.begin();
		for(;temp<ipAddr.end();temp++)
			if(strcmp(ip,temp->c_str())==0)
			{
				if(model==RANDOM)
				{
					widghtAll-=ipWeight[temp-ipAddr.begin()];
					std::vector<int>::iterator ptemp=ipWeight.begin();
					ptemp+=temp-ipAddr.begin();
					ipWeight.erase(ptemp);
				}
				ipAddr.erase(temp);
				temp--;
				flag=true;
			}
		return flag;
	}
	const char* getHashServer(const char* clientIp)
	{
		unsigned int one=0,two=0,three=0,four=0;
		if(0>=sscanf(clientIp,"%d.%d.%d.%d",&one,&two,&three,&four))
			return NULL;
		return ipAddr[(one+two+three+four)%ipAddr.size()].c_str();
	}
	inline Model getNowModel()
	{
		return model;
	}
}loadBanlance(LoadBalance::TEMPNO);
struct Config{
	struct Proxy{
		std::vector<std::string> host;
		LoadBalance load;
		Proxy(LoadBalance::Model model=LoadBalance::RANDOM):load(model){};
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
	std::string keyPath;
	std::string certPath;
	std::string passwd;
	std::vector<std::string> deletePath;
	std::vector<std::pair<std::string,std::string>> replacePath;
	std::vector<std::pair<std::string,std::string>> redirectPath;
	std::unordered_map<std::string,Proxy> proxyMap;
	Config():isLongConnect(true),isBack(false),isGuard(false),isLog(false),isAuto(true),isDebug(true),port(5200){};
}config;
void proxy(HttpServer& server,DealHttp& http,int soc)
{
	auto pathNow=server.getNowRoute();
	char url[128]={0};
	std::string strNow=pathNow->route;
	strNow+='*';
	http.getWildUrl(server.recText(),pathNow->route,url,128);
	if(config.proxyMap.find(strNow)!=config.proxyMap.end())
	{
		auto now=config.proxyMap[strNow].load;
		char ip[48]={0};
		int port=0,temp=0;
		if(now.getNowModel()!=LoadBalance::HASH)
			strNow=now.getServer();
		else
			strNow=now.getHashServer(ServerTcpIp::getPeerIp(soc,&temp));
		sscanf(strNow.c_str(),"%[^:]:%d",ip,&port);
		memset(server.getSenBuffer(),0,server.getMaxSenLen());
		DealHttp::Request req;
		http.analysisRequest(req,server.recText());
		req.askPath=url;
		req.head["Host"]=strNow;
		http.createAskRequest(req,server.getSenBuffer(),server.getMaxSenLen());
		ClientTcpIp client(ip,port);
		if(false==client.tryConnect())
		{
			http.gram.statusCode=DealHttp::STATUSNOFOUND;
			LogSystem::recordRequest("connect cliwrong",soc);
			return;
		}
		if(false==client.sendHost(server.getSenBuffer(),strlen((char*)server.getSenBuffer())))
		{
			http.gram.statusCode=DealHttp::STATUSNOFOUND;
			LogSystem::recordRequest("sen cliwrong",soc);
			return;
		}
		memset(server.getSenBuffer(),0,server.getMaxSenLen());
		int len=client.receiveHost(server.getSenBuffer(),server.getMaxSenLen());
		if(len<=0)
		{
			http.gram.statusCode=DealHttp::STATUSNOFOUND;
			LogSystem::recordRequest("rec cliwrong",soc);
			return;
		}
		client.disconnectHost();
		server.selfCreate(len);
	}
	else
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
}
class LoadConfig{
private:
	Json json;
	HttpServer* pserver;
	const char* error;
	std::vector<void (*)(const Config&,HttpServer&)> arr;
public:
	LoadConfig(const char* buffer):json(buffer),pserver(NULL),error(NULL)
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
	inline void configToServer(void (*pfunc)(const Config&,HttpServer&))
	{
		arr.push_back(pfunc);
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
		HttpServer server(config.port,config.isDebug,model);
		pserver=&server;
		configServer(server);
		if(pfunc!=NULL)
			pfunc(server);
		if(config.defaultFile.size()>0)
			server.run(config.defaultFile.c_str());
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
		if(config.isBack)
			ProcessCtrl::backGround();
		std::string pid=std::to_string(getpid());
		FileGet::writeToFile("./server.pid",pid.c_str(),pid.size());
		if(config.isGuard)
			ProcessCtrl::guard();
	}
	void configServer(HttpServer& server)
	{
		server.changeSetting(true,config.isLongConnect,config.isAuto,3);
		for(auto& now:config.deletePath)
			server.deletePath(now.c_str());
		for(auto& now:config.replacePath)
			server.loadStatic(now.first.c_str(),now.second.c_str());
		for(auto& now:config.redirectPath)
			server.redirect(now.first.c_str(),now.second.c_str());
		for(auto& now:config.proxyMap)
			server.all(now.first.c_str(),proxy);
		if(config.isLog)
			server.setLog(LogSystem::recordRequest,LogSystem::recordRequest);
		for(auto& now:arr)
			now(config,server);
#ifdef CPPWEB_OPENSSL
		if(FileGet::getFileLen(config.certPath.c_str())<=0||
		   FileGet::getFileLen(config.keyPath.c_str())<=0)
		{
			printf("cert file wrong\n");
			exit(0);
		}
		server.loadKeyCert(config.certPath.c_str(),config.keyPath.c_str(),config.passwd.size()==0?NULL:config.passwd.c_str());
#endif
	}
};
