#pragma once
//A header file that has not yet been completed
#include <string>
#include <vector>
#include <iostream>
class IniConfig{
public:
	struct Object{
		std::string value;
		std::string key;
		Object* pvalue; 
		Object* pnext;
		Object():pvalue(NULL),pnext(NULL){};
		Object& operator[](const char* key){
			if(pvalue==NULL){
				return IniConfig::npos;
			}else{
				auto& temp=*pvalue;
				while(1){
					if(temp.key==key){
						return temp;
					}else if(temp.pnext==NULL){
						break;
					}else{
						temp=*temp.pnext;
					}
				}
				return IniConfig::npos;
			}
		}
	};
	static Object npos;
private:
	std::string data;
	const char* error;
	Object root;
public:
	IniConfig(const std::string& data):data(data),error(NULL){
		root.pvalue=new Object;
		if(root.pvalue==NULL){
			error="malloc wrong";
			return;
		}
		deleteComment(this->data);
	};
	inline const char* lastError(){
		return error;
	}
public:
	bool analyseText(std::string& data){
		if(data[0]!='['){
			error="first char not section";
			return false;
		}
		return true;
	}
	void deleteComment(std::string& data){
		std::string temp;
		bool flag=false;
		for(auto ch:data){
			if(ch=='\n'){
				flag=false;
			}else if(ch==';'){
				flag=true;
			}
			if(!flag){
				temp+=ch;
			}
		}
		data=temp;
	}
};
IniConfig::Object IniConfig::npos;
