#pragma once
#include "test.h"
#include "../hpp/cppweb.h"
#include <cstdlib>
#include <fstream>
#include <unistd.h>

void* ConfigServer(void*){
	cppweb::HttpServer server(5200);
	//define stop function
	server.get("/stop",[](cppweb::HttpServer& server,cppweb::DealHttp& http,int){server.stopServer();});

	//txt ping-pong test
	server.get("/ping",[](cppweb::HttpServer&,cppweb::DealHttp& http,int){http.gram.txt(cppweb::DealHttp::STATUSOK, "pong");});

	//get query var
	server.get("/query*",[](cppweb::HttpServer&,cppweb::DealHttp& http,int){
			   auto val=http.req.queryValue("temp");
			   http.gram.txt(cppweb::DealHttp::STATUSOK,val);
			   });

	server.run();
	return nullptr;
}

INIT(CreateServer){
	cppweb::ThreadPool::createDetachPthread(nullptr,ConfigServer);
	return 0;
}
END(OnlineTest_Clean){
	system("rm temp");
}

std::string getCurlResult(const char* cmd){
	auto temp_no_use=system(cmd);
	std::fstream file("temp");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

TEST(ResufulApi, PingPong){
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/ping 2>/dev/null 1>temp"), "pong");
}

TEST(ResufulApi, QueryVar){
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/query?temp=hello 2>/dev/null 1>temp"), "hello");
}
