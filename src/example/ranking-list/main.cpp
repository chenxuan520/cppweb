#include <iostream>  
#include <string>
#include <map>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
multimap<int,string> tree;
void addCLi(HttpServer& server,DealHttp& http,int)
{
	http.req.analysisRequest(server.recText());
	auto name=http.req.formValue("name");
	auto scoStr=http.req.formValue("score");
	if(scoStr.size()==0)
	{
		http.gram.json(DealHttp::STATUSOK,Json::createJson({
													   {"status","wrong"},
													   {"error","cannot find score"}
													   }));
		return;
	}
	if(name.size()==0)
	{
		http.gram.json(DealHttp::STATUSOK,Json::createJson({
													   {"status","wrong"},
													   {"error","cannot find name"}
													   }));
		return;
	}
	http.req.urlDecode(name);
	int score=0;
	if(0>=sscanf(scoStr.c_str(),"%d",&score))
	{
		http.gram.json(DealHttp::STATUSOK,Json::createJson({
													   {"status","wrong"},
													   {"error","read score wrong"}
													   }));
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
	http.gram.json(DealHttp::STATUSOK,json());
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
	json["array"]=buf;
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

