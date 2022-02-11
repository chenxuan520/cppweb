#include <iostream>  
#include <string>
#include <map>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
multimap<int,string> tree;
void addCLi(HttpServer& server,DealHttp& http,int,DealHttp::Datagram& gram)
{
	char strSco[20]={0},name[50]={0};
	gram.typeFile=DealHttp::JSON;
	if(http.getKeyValue(server.recText(),"score",strSco,20,true)==NULL)
	{
		printf("get name wrong \n%s\n",(char*)server.recText());
		gram.statusCode=DealHttp::STATUSNOFOUND;
		gram.typeFile=DealHttp::NOFOUND;
		return;
	}
	if(http.getKeyValue(server.recText(),"name",name,50,true)==NULL)
	{
		printf("get name wrong \n%s\n",(char*)server.recText());
		gram.statusCode=DealHttp::STATUSNOFOUND;
		gram.typeFile=DealHttp::NOFOUND;
		return;
	}
	DealHttp::urlDecode(name);
	int score=0;
	if(0>=sscanf(strSco,"%d",&score))
	{
		printf("get score wrong \n%s\n",(char*)server.recText());
		gram.statusCode=DealHttp::STATUSNOFOUND;
		gram.typeFile=DealHttp::NOFOUND;
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
	char* str=json.createObject(200);
	json.addKeyVal(str,Json::STRING,"name",name);
	json.addKeyVal(str,Json::INT,"sco",score);
	json.addKeyVal(str,Json::STRING,"status","ok");
	gram.body=str;
}
void getList(HttpServer&,DealHttp&,int,DealHttp::Datagram& gram)
{
	auto begin=tree.begin();
	gram.typeFile=DealHttp::JSON;
	char* buf[9]={0};
	Json json;
	auto str1=json.createObject(400);
	json.addKeyVal(str1,Json::STRING,"status","ok");
	unsigned int i=0;
	while(begin!=tree.end())
	{
		printf("%d %s\n",begin->first,begin->second.c_str());
		buf[i]=json.createObject(100);
		json.addKeyVal(buf[i],Json::STRING,"name",begin->second.c_str());
		json.addKeyVal(buf[i],Json::INT,"score",begin->first);
		cout<<buf[i]<<endl;
		begin++;
		i++;
	}
	auto arr=json.createArray(200,Json::OBJ,i,buf);
	json.addKeyVal(str1,Json::ARRAY,"array",arr);
	gram.body=str1;
}
int main()  
{  
	HttpServer server(5200,true);
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

