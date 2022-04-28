#pragma once
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
* change time:2022-04-02 19:17:21
* description:http and https ForwardProxy,donnot change or use this class
***********************************************/
class ForwardProxy{
private:
	static void *httpsTunnel(void* arg)
	{
		std::pair<int, int> &_arg = *(std::pair<int, int> *)arg;
		int soc = _arg.first;
		char *buffer = (char *)malloc(sizeof(char) * 1024 * 512);
		if (buffer == NULL)
		{
			printf("malloc error\n");
			return NULL;
		}
		int len = 0;
		while (true)
		{
			memset(buffer, 0, 1024 * 512);
			len = recv(_arg.second, buffer, sizeof(char) * 1024 * 512, 0);
			if (len <= 0)
				break;
			if (send(soc, buffer, len, 0) <= 0)
				break;
		}
		free(buffer);
		return NULL;
	}
public:
	static void httpProxy(HttpServer &server, DealHttp &http, int)
	{
		DealHttp::Request req;
		http.analysisRequest(req, server.recText());
		char top[128] = {0}, end[128] = {0}, ip[32] = {0};
		DealHttp::dealUrl(req.askPath.c_str(), top, end, 128, 128);
		req.askPath = end;
		http.createAskRequest(req, server.getSenBuffer(), server.getMaxSenLen());
		ClientTcpIp::getDnsIp(top, ip, 32);
		char *sen = (char *)server.getSenBuffer();
		ClientTcpIp client(ip, 80);
		if (client.tryConnect() == false)
		{
			http.gram.statusCode = DealHttp::STATUSNOFOUND;
			return;
		}
		if (client.sendHost(sen, strlen(sen)) <= 0)
		{
			http.gram.statusCode = DealHttp::STATUSNOFOUND;
			return;
		}
		int len = 0;
		std::string rec;
		if ((len = client.receiveHost(rec)) <= 0)
		{
			http.gram.statusCode = DealHttp::STATUSNOFOUND;
			return;
		}
		if (server.getMaxSenLen() < rec.size())
		{
			server.enlagerSenBuffer();
		}
		memcpy(server.getSenBuffer(), rec.c_str(), rec.size());
		server.selfCreate(rec.size());
	}
	static void httpsProxy(HttpServer &server, DealHttp &http, int soc)
	{
		DealHttp::Request req;
		int port = 0;
		http.analysisRequest(req, server.recText());
		char buffer[2048] = {0}, ip[32] = {0};
		sscanf(req.askPath.c_str(), "%[^:]:%d", buffer, &port);
		ClientTcpIp::getDnsIp(buffer, ip, 32);
		ClientTcpIp client(ip, port);
		if (false == client.tryConnect())
		{
			http.gram.statusCode = DealHttp::STATUSNOFOUND;
			printf("connect error\n");
			return;
		}
		auto len = sprintf((char *)buffer, "HTTP/1.1 200 Connection Established\r\n"
										   "Connection:close\r\n\r\n");
		server.httpSend(soc, buffer, len);
		std::pair<int, int> now = {soc, client.getSocket()};
		ThreadPool::createDetachPthread(&now,httpsTunnel);
		char *temp = (char *)server.getSenBuffer();
		int maxLen = server.getMaxSenLen();
		while (true)
		{
			len = server.httpRecv(soc, temp, maxLen);
			if (len <= 0)
				break;
			len = client.sendHost(temp, len);
			if (len <= 0)
				break;
		}
		server.selfCreate(-1);
	}
};
