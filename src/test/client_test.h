#pragma once
#include "test.h"
#include "../hpp/cppweb.h"
using namespace std;
using namespace cppweb;

TEST(ClientTcpIp, TcpConnect){
	ClientTcpIp cli(cli.getDnsIp("chenxuanweb.top"),80);
	MUST_EQUAL(cli.lastError(), nullptr);
	MUST_TRUE(cli.tryConnect(),"connext error");
	const char* ask="GET / HTTP/1.1";
	int data_num=cli.sendHost(ask,strlen(ask));
	MUST_TRUE(data_num>0, "data_num is "<<data_num);
	string result;
	data_num=cli.receiveHost(result);
	MUST_TRUE(data_num>0, "data_num is "<<data_num);

	DealHttp::Response res;
	MUST_TRUE(res.analysisResponse(result), "analysisResponse wrong "<<res.error);

	MUST_EQUAL(res.statusCode, 302);
}
