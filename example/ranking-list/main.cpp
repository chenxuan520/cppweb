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
	Json json;
	char* str=json.createObject();
	json.addKeyVal(str,Json::STRING,"name",name);
	json.addKeyVal(str,Json::INT,"sco",score);
	json.addKeyVal(str,Json::STRING,"status","ok");
	http.gram.body=str;
}
void getList(HttpServer&,DealHttp& http,int)
{
	auto begin=tree.begin();
	http.gram.typeFile=DealHttp::JSON;
	char* buf[9]={0};
	Json json;
	auto str1=json.createObject();
	json.addKeyVal(str1,Json::STRING,"status","ok");
	unsigned int i=0;
	while(begin!=tree.end())
	{
		printf("%d %s\n",begin->first,begin->second.c_str());
		buf[i]=json.createObject();
		json.addKeyVal(buf[i],Json::STRING,"name",begin->second.c_str());
		json.addKeyVal(buf[i],Json::INT,"score",begin->first);
		cout<<buf[i]<<endl;
		begin++;
		i++;
	}
	auto arr=json.createArray(Json::OBJ,i,buf);
	json.addKeyVal(str1,Json::ARRAY,"array",arr);
	http.gram.body=str1;
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

