#include "../class/ini.h"
#include "../hpp/cppweb.h"
using namespace std;
using namespace cppweb;

int main()
{
	auto config=FileGet::getFileString("./config_test.ini");
	IniConfig ini(config);
	auto result=ini.getAnalyseResult();
	config="";
	config=ini.createConfig(result);
	if(result.find("mysql")!=result.end()){
		for(auto& [key,value]:result["mysql"]){
			cout<<"key: "<<key<<" val: "<<value.toString()<<endl;
		}
	}
	cout<<config<<endl;
	return 0;
}
