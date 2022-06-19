#pragma once
#include <unordered_map>
#include <string>
class ArgcDeal{
private:
	char** argv;
	int argc;
	std::unordered_map<std::string,void (*)(std::string,void*)> hashMap;
public:
	ArgcDeal(char** argv,int argc):argv(argv),argc(argc){}
	void registerVari(std::string variName,void (*pfunc)(std::string,void*)){
		hashMap.insert({variName,pfunc});
	}
};
