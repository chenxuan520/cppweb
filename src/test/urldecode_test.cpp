#include <iostream>
#include <vector>
#include <string>
#include "../hpp/cppweb.h"
using namespace std;

int main()
{
	char temp[100]="/ji%E7%A7%AF%E6%9E%81";
	cppweb::DealHttp::urlDecode(temp);
	std::cout << temp << std::endl;
	return 0;
}
