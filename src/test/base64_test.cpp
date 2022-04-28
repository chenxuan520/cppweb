#include <iostream>
#include "../hpp/encrypt.h"
using namespace std;
int main()
{
	string temp;
	cin>>temp;
	auto result=Base64::Encode(temp.c_str(),temp.size());
	cout<<"result:"<<result<<endl;
	result=Base64::Decode(result.c_str(),result.size());
	cout<<"result:"<<result<<endl;
	return 0;
}
