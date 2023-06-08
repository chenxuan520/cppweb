/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2023-04-14 20:56:46
* description: this is a simple C++ testing framework
***********************************************/
#pragma once
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>
#include <functional>

//base class
class _test_base{
public:
	bool result=true;
	static std::vector<std::pair<_test_base*, std::string>> test_arr;
	static int success;
	static int fail;
	virtual void TestBody(){};
};
int _test_base::success=0;
int _test_base::fail=0;
std::vector<std::pair<_test_base*, std::string>> _test_base::test_arr;

// std out or stderr
#define TESTSTDOUT(text) std::cout<<text<<std::endl;
#define TESTSTDERR(text) std::cerr<<text<<std::endl;

//print kinds of color
#define TESTRED(text)     "\033[31m"<<text<<"\033[0m"
#define TESTGREEN(text)   "\033[32m"<<text<<"\033[0m"
#define TESTYELLOW(text)  "\033[33m"<<text<<"\033[0m"
#define TESTBLUE(text)    "\033[34m"<<text<<"\033[0m"
#define TESTCAR(text)     "\033[35m"<<text<<"\033[0m"
#define TESTCYAN(text)    "\033[36m"<<text<<"\033[0m"

//util macro
#define CPPWEBCONNECTSTR(...) #__VA_ARGS__
#define _CLASS_FAIL_ this->fail++;this->success--;this->result=false
#define _FILE_LINE_MSG_ __FILE__<<":"<<__LINE__

//test name
#define CPPWEB_TEST_NAME_CREATE(test_group,test_name) test_group##test_name##_create
#define CPPWEB_TEST_NAME(test_group,test_name) test_group##test_name##_cppweb
#define CPPWEB_TEST_INIT_NAME_CREATE(init_name) init_name##_init##_create
#define CPPWEB_TEST_INIT_NAME(init_name) init_name##_init
#define CPPWEB_TEST_END_NAME_CREATE(end_name) end_name##_end##_create
#define CPPWEB_TEST_END_NAME(end_name) end_name##_end

//test init function,run before all test example
#define INIT(init_name)                                                              \
	int CPPWEB_TEST_INIT_NAME(init_name)();                                          \
	auto CPPWEB_TEST_INIT_NAME_CREATE(init_name)=CPPWEB_TEST_INIT_NAME(init_name)(); \
	int CPPWEB_TEST_INIT_NAME(init_name)()

//test end function,run after all test example
#define END(end_name)                                                     \
	class CPPWEB_TEST_END_NAME(end_name){                                 \
	public:                                                               \
		~CPPWEB_TEST_END_NAME(end_name)();                                \
	};                                                                    \
	CPPWEB_TEST_END_NAME(end_name) CPPWEB_TEST_END_NAME_CREATE(end_name); \
	CPPWEB_TEST_END_NAME(end_name)::~CPPWEB_TEST_END_NAME(end_name)()

//test function for users
#define TEST(test_group,test_name)                                                        \
	class CPPWEB_TEST_NAME(test_group,test_name):public _test_base{                       \
	public:                                                                               \
		CPPWEB_TEST_NAME(test_group,test_name)(){                                         \
			test_arr.push_back({this,CPPWEBCONNECTSTR(test_group test_name)});            \
			this->success++;                                                              \
		}                                                                                 \
		void TestBody();                                                                  \
	};                                                                                    \
	CPPWEB_TEST_NAME(test_group,test_name) CPPWEB_TEST_NAME_CREATE(test_group,test_name); \
	void CPPWEB_TEST_NAME(test_group,test_name)::TestBody()

//some function for debug and judge
#define SKIP()      TESTSTDOUT(TESTYELLOW("[SKIP] in ["<<_FILE_LINE_MSG_<<"] : "));return;
#define DEBUG(text) TESTSTDOUT(TESTYELLOW("[DEBUG] in ["<<_FILE_LINE_MSG_<<"] : "<<text))
#define ERROR(text) TESTSTDERR(TESTCAR("[ERROR] in ["<<_FILE_LINE_MSG_<<"] : "<<text))\
	_CLASS_FAIL_
#define FATAL(text) TESTSTDERR(TESTRED("[FATAL] in ["<<_FILE_LINE_MSG_<<"] : "<<text))\
	_CLASS_FAIL_;return;
#define PANIC(text) TESTSTDERR(TESTRED("[PANIC] in ["<<_FILE_LINE_MSG_<<"] : "<<text));exit(-1);
#define EXPECT_EQ(result,expect)\
	if (result!=expect){FATAL(CPPWEBCONNECTSTR(result want get expect but get)<<" "<<result)}
#define MUST_EQUAL(result,expect)\
	if(result!=expect){FATAL(result<<" "<<CPPWEBCONNECTSTR(!= expect));}
#define MUST_TRUE(flag,text)\
	if(!(flag)){FATAL(text);}

//for argc message
static void (*__test_argc_funcpr__)(int argc,char* argv[])=nullptr;
static void __test_deal_argc__(int argc,char** argv);
#define ARGC_FUNC static auto __test__temp__argc__=(__test_argc_funcpr__=__test_deal_argc__);\
												   void __test_deal_argc__(int argc,char* argv[])

//the main function
#define RUN                                                           \
	int main(int argc,char* argv[]){                                  \
		if (__test_argc_funcpr__!=nullptr) {                          \
			__test_argc_funcpr__(argc,argv);                          \
		}                                                             \
		_test_base base;                                              \
		for (int i = 0; i < base.test_arr.size(); i++) {              \
			TESTSTDOUT(TESTCYAN("Runing:"<<base.test_arr[i].second)); \
			base.test_arr[i].first->TestBody();                       \
			if(base.test_arr[i].first->result){                       \
				TESTSTDOUT(TESTGREEN("Result:PASS"));                 \
			}else{                                                    \
				TESTSTDOUT(TESTRED("Result:Fail"));                   \
			}                                                         \
			std::cout<<std::endl;                                     \
		}                                                             \
		TESTSTDOUT(TESTBLUE("Total Run:"<<base.success+base.fail))    \
		TESTSTDOUT(TESTBLUE("Success Run:"<<base.success))            \
		TESTSTDERR(TESTBLUE("Fail Run:"<<base.fail))                  \
		return base.fail;                                             \
	}
