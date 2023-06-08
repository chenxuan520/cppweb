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

	server.run();
	return nullptr;
}

INIT(CreateServer){
	cppweb::ThreadPool::createDetachPthread(nullptr,ConfigServer);
	return 0;
}

TEST(ResufulApi, PingPong){
	auto data=system("curl http://127.0.0.1:5200/ping 2>/dev/null 1>temp");
	std::fstream file("temp");
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	MUST_EQUAL(content, "pong");
}

END(OnlineTest_Clean){
	system("rm temp");
}
