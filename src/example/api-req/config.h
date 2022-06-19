#pragma once
#include <iostream>
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
class Config{
private:
	Json json;
public:
	struct Host{
		string ip;
		unsigned port;
	};
	const char* error;
	Host host;
	DealHttp::Request req;
	string body;
	Config(string configFile){
		error=NULL;
		auto text=FileGet::getFileString(configFile.c_str());
		if(text.size()==0){
			error="file size wrong";
			return;
		}
		auto flag=json.analyseText(text.c_str());
		if(flag==false){
			error=json.lastError();
			return;
		}
		auto root=json.getRootObj();
		if(root["host"]["ip"]!=Json::npos){
			host.ip=root["host"]["ip"].strVal;
			char temp[32]={0};
			if(false==ClientTcpIp::getDnsIp(host.ip.c_str(),temp,32)){
				error="dns wrong";
				return;
			}
			host.ip=temp;
		}else{
			error="ip wrong";
			return;
		}
		if(root["host"]["port"]!=Json::npos){
			host.port=root["host"]["port"].intVal;
		}else{
			error="port wrong";
			return;
		}
		if(root["req"]["path"]!=Json::npos){
			req.askPath=root["req"]["path"].strVal;
		}else{
			error="path wrong";
			return;
		}
		if(root["req"]["method"]!=Json::npos){
			req.method=root["req"]["method"].strVal;
		}else{
			error="method wrong";
			return;
		}
		if(root["req"]["version"]!=Json::npos){
			req.version=root["req"]["version"].strVal;
		}
		if(root["req"]["head"]!=Json::npos){
			auto now=root["req"]["head"].nextObj;
			while(now!=NULL){
				req.head.insert({now->key,now->strVal});
				now=now->nextObj;
			}
			if(req.head.find("Host")==req.head.end()){
				req.head.insert({"Host",root["host"]["ip"].strVal});
			}
		}else{
			error="head wrong";
			return;
		}
		if(root["req"]["body"]!=Json::npos){
			req.body=root["req"]["body"].excapeChar().strVal.c_str();
		}else if(root["req"]["body_file"]!=Json::npos){
			body=FileGet::getFileString(root["req"]["body_file"].strVal.c_str());
			req.body=body.c_str();
		}else{
			error="body wrong";
			return;
		}
	}
};
