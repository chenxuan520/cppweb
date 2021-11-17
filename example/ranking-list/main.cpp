#include <iostream>  
#include <string>
#include <map>
#include "../../lib/server.h"
#include "../../lib/http.h"
using namespace std;
multimap<int,string> tree;
void addCLi(DealHttp & http, HttpServer & server, int , void * sen, int & len)
{
	char strSco[20]={0},name[30]={0};
	if(http.getKeyValue(server.recText(),"score",strSco,20)==NULL)
	{
		printf("get name wrong \n%s\n",server.recText());
		http.createSendMsg(DealHttp::NOFOUND,(char*)sen,NULL,&len);
		return;
	}
	if(http.getKeyValue(server.recText(),"name",name,30)==NULL)
	{
		printf("get name wrong \n%s\n",server.recText());
		http.createSendMsg(DealHttp::NOFOUND,(char*)sen,NULL,&len);
		return;
	}
	int score=0;
	if(0>=sscanf(strSco,"%d",&score))
	{
		printf("get score wrong \n%s\n",server.recText());
		http.createSendMsg(DealHttp::NOFOUND,(char*)sen,NULL,&len);
		return;
	}
	if(tree.size()<=5)
	{
		printf("add success\n");
		tree.insert(pair<int,string>{score,name});
	}
	else
	{
		if(score>tree.begin()->first)
		{
			tree.erase(tree.begin());
			tree.insert(pair<int,string>{score,name});
		}
	}
	Json json;
	json.init(200);
	json.addKeyValue("name",name);
	json.addKeyValInt("sco",score);
	json.endJson();
	json.jsonToFile("temp");
	http.createSendMsg(DealHttp::JSON,(char*)sen,"temp",&len);
}
void getList(DealHttp & http, HttpServer & server, int , void * sen, int & len)
{
	auto begin=tree.begin();
	Json json;
	json.init(300);
	while(begin!=tree.end())
	{
		printf("%d %s\n",begin->first,begin->second.c_str());
		json.addKeyValInt("score",begin->first);
		json.addKeyValue("name",begin->second.c_str());
		begin++;
	}
	json.endJson();
	json.jsonToFile("temp");
	http.createSendMsg(DealHttp::JSON,(char*)sen,"temp",&len);
}
int main()  
{  
	HttpServer server(5200);
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.post(HttpServer::ONEWAY,"/add",addCLi);
	server.get(HttpServer::ONEWAY,"/list",getList);
	server.run(1,4000,"index.html");
    return 0; 
}  

