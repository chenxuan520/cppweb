#include <iostream>
#include "../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
void func(HttpServer&,DealHttp& http,int)
{
	unordered_map<string,string> hash;
	hash["name"]="chenxuan";
	hash["lover"]="xiaozhu";
	http.gram.body=FileGet::renderHtml("./test.html",hash);
	http.gram.typeFile=DealHttp::HTML;
}
int main()
{
	DealHttp http;
	cout<<http.findBackString("haha","haha : ok o")<<endl;;
	return 0;
}
