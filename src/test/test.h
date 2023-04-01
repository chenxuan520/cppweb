#pragma once
#include <iostream>
#include <utility>
#include <vector>

class test_base{
public:
	bool result=true;
	static std::vector<std::pair<test_base*, std::string>> test_arr;
	static int success;
	static int fail;
	virtual void TestBody(){};
};
int test_base::success=0;
int test_base::fail=0;
std::vector<std::pair<test_base*, std::string>> test_base::test_arr;

#define CONNECTSTR(...) #__VA_ARGS__

#define CPPWEB_TEST_NAME_CREATE(test_group,test_name) test_group##test_name##_create
#define CPPWEB_TEST_NAME(test_group,test_name) test_group##test_name##_cppweb

#define TEST(test_group,test_name)                                                        \
	class CPPWEB_TEST_NAME(test_group,test_name):public test_base{                        \
	public:                                                                               \
		CPPWEB_TEST_NAME(test_group,test_name)(){                                         \
			test_arr.push_back({this,CONNECTSTR(test_group test_name)});                  \
			this->success++;                                                              \
		}                                                                                 \
		void TestBody();                                                                  \
	};                                                                                    \
	CPPWEB_TEST_NAME(test_group,test_name) CPPWEB_TEST_NAME_CREATE(test_group,test_name); \
	void CPPWEB_TEST_NAME(test_group,test_name)::TestBody()

#define TESTRED(text)     std::cout<<"\033[31m"<<text<<"\033[0m"<<std::endl;
#define TESTGREEN(text)   std::cout<<"\033[32m"<<text<<"\033[0m"<<std::endl;
#define TESTYELLOW(text)  std::cout<<"\033[33m"<<text<<"\033[0m"<<std::endl;
#define TESTBLUE(text)    std::cout<<"\033[34m"<<text<<"\033[0m"<<std::endl;
#define TESTCAR(text)     std::cout<<"\033[35m"<<text<<"\033[0m"<<std::endl;
#define TESTCYAN(text)    std::cout<<"\033[36m"<<text<<"\033[0m"<<std::endl;

#define FATAL(text) TESTRED("[FATAL]:"<<text)\
	this->fail++;this->success--;this->result=false;return;
#define MUST_EQUAL(one,two)\
	if(one!=two){FATAL(CONNECTSTR(one != two));}
#define MUST_TRUE(flag,text)\
	if(!(flag)){FATAL(text);}
#define SKIP()      TESTYELLOW("[SKIP]");return;
#define DEBUG(text) TESTYELLOW("[DEBUG]:"<<text)
#define ERROR(text) TESTCAR("[ERROR]:"<<text)\
	this->fail++;this->success--;this->result=false

#define RUN                                                                      \
	int main(){                                                                  \
		test_base base;                                                          \
		for (int i = 0; i < base.test_arr.size(); i++) {                         \
			TESTGREEN("Runing:"<<base.test_arr[i].second);                       \
			base.test_arr[i].first->TestBody();                                  \
			TESTGREEN("Result:"<<std::boolalpha<<base.test_arr[i].first->result) \
			std::cout<<std::endl;                                                \
		}                                                                        \
		TESTBLUE("Total Run:"<<base.success+base.fail)                           \
		TESTBLUE("Success Run:"<<base.success)                                   \
		TESTBLUE("Fail Run:"<<base.fail)                                         \
		return 0;                                                                \
	}
