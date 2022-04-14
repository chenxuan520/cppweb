#include <iostream>  
#include <string>
#include <map>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
multimap<int,string> tree;
void addCLi(HttpServer& server,DealHttp& http,int)
{
	char strSco[20]={0},name[50]={0};
	http.gram.typeFile=DealHttp::JSON;
	if(http.getKeyValue(server.recText(),"score",strSco,20,true)==NULL)
	{
		printf("get name wrong \n%s\n",(char*)server.recText());
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
		http.gram.typeFile=DealHttp::NOFOUND;
		return;
	}
	if(http.getKeyValue(server.recText(),"name",name,50,true)==NULL)
	{
		printf("get name wrong \n%s\n",(char*)server.recText());
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
		http.gram.typeFile=DealHttp::NOFOUND;
		return;
	}
	DealHttp::urlDecode(name);
	int score=0;
	if(0>=sscanf(strSco,"%d",&score))
	{
		printf("get score wrong \n%s\n",(char*)server.recText());
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
		http.gram.typeFile=DealHttp::NOFOUND;
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
	Json json={
		{"name",name},
		{"sco",score},
		{"status","ok"}
	};
	http.gram.body=json();
}
void getList(HttpServer&,DealHttp& http,int)
{
	auto begin=tree.begin();
	http.gram.typeFile=DealHttp::JSON;
	vector<Json::Node> buf;
	Json json={{"status","ok"}};
	while(begin!=tree.end())
	{
		Json::Node node={
			{"name",begin->second},
			{"score",begin->first}
		};
		printf("%d %s\n",begin->first,begin->second.c_str());
		buf.push_back(node);
		begin++;
	}
	json[ "array" ]=buf;
	http.gram.body=json();
}
int main()  
{  
	HttpServer server(5200,true);
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
	server.post("/add",addCLi);
	server.get("/list",getList);
	server.run("api.html");
    return 0; 
}  

