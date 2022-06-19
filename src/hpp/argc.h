#pragma once
#include <unordered_map>
#include <iostream>
#include <string>
namespace cppweb{
class ArgcDeal{
public:
	struct AppMsg{
		std::string name;
		std::string version;
		std::string usage;
		int (*pfunc)(ArgcDeal&);
	};
private:
	char** argv;
	int argc;
	std::unordered_map<std::string,std::string> argAll;
	std::unordered_map<std::string,std::string> message;
public:
	AppMsg app;
public:
	ArgcDeal(int argc,char** argv):argv(argv),argc(argc){
		for(int i=1;i<argc;i++){
			char* temp=argv[i];
			while(*temp=='-')
				temp++;
			std::string now=temp;
			if(now.find("=")==now.npos){
				argAll.insert({temp,""});
			}else{
				std::string key=now.substr(0,now.find("="));
				std::string value=now.substr(now.find("=")+1);
				argAll.insert({key,value});
			}
		}
	}
	std::string getVari(std::string key){
		if(argAll.find(key)==argAll.end()){
			return "";
		}else{
			return argAll[key];
		}
	}
	void setVari(std::string variName,std::string usage,bool must=false){
		std::string temp="\t,no must";
		if(must)
			temp="\t,must";
		message.insert({"--"+variName+"=(variable)",usage+temp});
		if(must&&argAll.find(variName)==argAll.end()){
			app.pfunc=NULL;
		}
	}
	bool getOption(std::string name){
		return argAll.find(name)!=argAll.end();
	}
	void setOption(std::string option,std::string usage,bool must=false){
		std::string temp="\t,no must";
		if(must)
			temp="\t,must";
		message.insert({"-"+option,usage+temp});
		if(must&&argAll.find(option)==argAll.end()){
			app.pfunc=NULL;
		}
	}
	int run(){
		if(app.pfunc==NULL||argAll.find("h")!=argAll.end()||argAll.find("help")!=argAll.end()){
			return msgPrint();
		}else{
			return app.pfunc(*this);
		}
	}
	int msgPrint(){
		printf("app name:\n");
		printf("\t%s  %s\n",app.name.c_str(),app.version.c_str());
		printf("usage:\n");
		printf("\t%s\n",app.usage.c_str());
		printf("options:\n");
		for(auto iter:message){
			printf("\t%s\t\t%s\n",iter.first.c_str(),iter.second.c_str());
		}
		printf("\t-h,--help\t\tget help of app\n");
		return 0;
	}
};
};
