#pragma once
#include <iostream>

#define TEST(test_group,test_name) TODO
class TEST{
private:
public:
	void EQUAL_CONTINUE();
	void EQUAL_FAIL();
	//print some thing;
	template<typename T>
		void DEBUG(T print){
			std::cout<<"DEBUG:"<<print<<std::endl;
		}
	template<typename T>
		void ERROR(T print){
			std::cout<<"ERROR:"<<print<<std::endl;
		}
	template<typename T>
		void FATAL(T print){
			std::cout<<"FATAL:"<<print<<std::endl;
		}
};
