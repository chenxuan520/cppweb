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
	server.get("/query",[](cppweb::HttpServer&,cppweb::DealHttp& http,int){
			   auto val=http.req.queryValue("temp");
			   http.gram.txt(cppweb::DealHttp::STATUSOK,val);
			   });

	//post from val
	server.post("/postvar",[](cppweb::HttpServer&,cppweb::DealHttp& http,int){
			   auto val=http.req.formValue("temp");
			   http.gram.txt(cppweb::DealHttp::STATUSOK,val);
			   });

	//get wild route pair
	server.get("/wild/*",[](cppweb::HttpServer&,cppweb::DealHttp& http,int){
			   http.gram.txt(cppweb::DealHttp::STATUSOK, "wild");
			   });

	//trye cookie read
	server.get("/read_cookie",[](cppweb::HttpServer&,cppweb::DealHttp& http,int){
			   auto val=http.req.getCookie("cookie_try");
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

std::string getCurlResult(const std::string& cmd){
	std::string cmd_fin=cmd+" 2>/dev/null 1>temp";
	auto temp_no_use=system(cmd_fin.c_str());
	std::fstream file("temp");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

TEST(ResufulApi, PingPong){
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/ping"), "pong");
}

TEST(ResufulApi, QueryVar){
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/query?temp=hello"), "hello");
}

TEST(ResufulApi, PostFormVar){
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/postvar -d 'temp=post' -X POST"), "post");
}

TEST(ResufulApi, RouteWild){
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/wild/one"), "wild");
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/wild/two"), "wild");
}

TEST(ResufulApi, WrongRoute){
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/empty/one"),"404 no found");
}

TEST(ResufulApi, CookieRead){
	MUST_EQUAL(getCurlResult("curl http://127.0.0.1:5200/read_cookie --cookie 'cookie_try=val'"), "val");
}
