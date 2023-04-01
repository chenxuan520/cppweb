#pragma once
#include "test.h"
#include "../hpp/cppweb.h"
using namespace std;
using namespace cppweb;

TEST(DealHttp, urldecode){
	std::string temp="/ji%E7%A7%AF%E6%9E%81";
	cppweb::DealHttp::Request::urlDecode(temp);
	MUST_EQUAL(temp, "/ji积极");
}
