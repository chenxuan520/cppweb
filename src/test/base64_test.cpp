#include <iostream>
#include "../hpp/encrypt.h"
#include <openssl/md5.h>
#include <openssl/evp.h>
using namespace std;
int main()
{
	string temp;
	cin>>temp;
	auto re2=Base64::encode(temp.c_str(),temp.size());
	cout<<"result2:"<<re2<<endl;
	re2=Base64::decode(re2.c_str(),re2.size());
	cout<<"result2:"<<re2<<endl;
	auto result=Md5::encode(temp.c_str(),temp.size());
	cout<<result<<endl;
	return 0;
}
