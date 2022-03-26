#pragma once
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-19 11:55:48
* description:this file is for server config,do not change it
***********************************************/
#include "./cppweb.h"
using namespace cppweb;
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-17 19:20:18
* description:class for Reverse proxy
* example:{ LoadBalance load(RANDOM)
* load.addServer("127.0.0.1:5200")
* char* getIp=load.getServer();
* }
***********************************************/
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
	bool isLongConnect;
	bool isBack;
	bool isGuard;
	bool isLog;
	bool isAuto;
	bool isDebug;
	int port;
	int defaultMemory;
	int threadNum;
	std::string defaultFile;
	std::string logPath;
	std::string model;
	std::string keyPath;
	std::string certPath;
	std::string passwd;
	std::vector<std::string> deletePath;
	std::vector<std::pair<std::string,std::string>> replacePath;
	std::vector<std::pair<std::string,std::string>> redirectPath;
	std::unordered_map<std::string,Proxy> proxyMap;
	Config():isLongConnect(true),isBack(false),isGuard(false),isLog(false),isAuto(true),isDebug(true),port(5200),defaultMemory(1),threadNum(0){};
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
	http.getWildUrl(server.recText(),pathNow->route,url,128);
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
// description:deal kill signal
static void _dealSignalKill(int)
{
	LogSystem::recordRequest("server stop ",0);
#ifndef _WIN32
	if(ProcessCtrl::childPid!=0)
		kill(ProcessCtrl::childPid,2);
#endif
	exit(0);
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
	HttpServer* pserver;
	const char* error;
	std::vector<std::pair<std::string,void (*)(Json::Object*,HttpServer&)>> arr;
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
		pfunc(root[key],_config);
	}
	void findConfig(const char* key,void (*pfunc)(Json::Object*,HttpServer&))
	{
		arr.push_back(std::pair<std::string,void (*)(Json::Object*,HttpServer&)>{key,pfunc});
	}
	void runServer(void (*pfunc)(HttpServer&)=NULL)
	{
		configProcess();
		HttpServer::RunModel model;
		if(_config.model=="THREAD")
			model=HttpServer::THREAD;
		else if(_config.model=="FORK")
			model=HttpServer::FORK;
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
		if(_config.model=="THREAD"&&_config.threadNum!=0)
			server.changeModel(HttpServer::THREAD,_config.threadNum);
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
			server.setLog(LogSystem::recordRequest,LogSystem::recordRequest);
#ifndef _WIN32
			signal(SIGINT,_dealSignalKill);
			signal(SIGQUIT,_dealSignalKill);
			signal(SIGTERM,_dealSignalKill);
#endif
		}
		for(auto& now:arr)
			now.second(json[now.first.c_str()],server);
#ifdef CPPWEB_OPENSSL
		if(FileGet::getFileLen(_config.certPath.c_str())<=0||
		   FileGet::getFileLen(_config.keyPath.c_str())<=0)
		{
			printf("cert file wrong\n");
			exit(0);
		}
		server.loadKeyCert(_config.certPath.c_str(),_config.keyPath.c_str(),_config.passwd.size()==0?NULL:_config.passwd.c_str());
#endif
	}
};
