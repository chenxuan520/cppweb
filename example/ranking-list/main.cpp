#include <iostream>  
#include <string>
#include <map>
#include "../../lib/server.h"
#include "../../lib/http.h"
using namespace std;
using namespace cppweb;
multimap<int,string> tree;
void addCLi(DealHttp & http, HttpServer & server, int , void * sen, int & len)
{
	memset(sen,0,sizeof(char)*10000);
	char strSco[20]={0},name[50]={0};
	if(http.getKeyValue(server.recText(),"score",strSco,20,true)==NULL)
	{
		printf("get name wrong \n%s\n",(char*)server.recText());
		http.createSendMsg(DealHttp::NOFOUND,(char*)sen,10000,NULL,&len);
		return;
	}
	if(http.getKeyValue(server.recText(),"name",name,50,true)==NULL)
	{
		printf("get name wrong \n%s\n",(char*)server.recText());
		http.createSendMsg(DealHttp::NOFOUND,(char*)sen,10000,NULL,&len);
		return;
	}
	DealHttp::urlDecode(name);
	int score=0;
	if(0>=sscanf(strSco,"%d",&score))
	{
		printf("get score wrong \n%s\n",(char*)server.recText());
		http.createSendMsg(DealHttp::NOFOUND,(char*)sen,10000,NULL,&len);
		return;
	}
	if(tree.size()<5)
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
	json.jsonToFile("temp");
	http.createSendMsg(DealHttp::JSON,(char*)sen,10000,"temp",&len);
}
void getList(DealHttp & http, HttpServer & , int , void * sen, int & len)
{
	memset(sen,0,sizeof(char)*10000);
	auto begin=tree.begin();
	char* buf[9]={0};
	for(unsigned int i=0;i<9;i++)
	{
		buf[i]=(char*)malloc(sizeof(char)*100);
		memset(buf[i],0,sizeof(char)*100);
	}
	Json json;
	json.init(600);
	Json::Object score,name,array;
	score.type=Json::INT;
	name.type=Json::STRING;
	score.key="score";
	name.key="name";
	array.type=Json::ARRAY;
	array.key="array";
	array.arrTyp=Json::STRUCT;
	array.arrLen=tree.size();
	array.array=(void**)buf;
	unsigned int i=0;
	while(begin!=tree.end())
	{
		printf("%d %s\n",begin->first,begin->second.c_str());
		score.valInt=begin->first;
		name.valStr=begin->second.c_str();
		json.createObject(buf[i],100,score);
		json.createObject(buf[i],100,name);
		cout<<buf[i]<<endl;
		begin++;
		i++;
	}
	json.addOBject(array);
	json.addKeyValInt("len",tree.size());
	json.jsonToFile("temp");
	http.createSendMsg(DealHttp::JSON,(char*)sen,10000,"temp",&len);
	for(unsigned int i=0;i<9;i++)
		free(buf[i]);
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
	server.run(1,4000,"api.html");
    return 0; 
}  

