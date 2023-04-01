#include "test.h"
#include "../hpp/cppweb.h"
#include <cstdio>

TEST(TestDealHttp, urldecode){
	std::string temp="/ji%E7%A7%AF%E6%9E%81";
	cppweb::DealHttp::Request::urlDecode(temp);
	DEBUG(temp);
	MUST_EQUAL(temp, "/ji积极");
}

TEST(TestForHeader, RunTest0){
	MUST_EQUAL(1, 1);
}

RUN
