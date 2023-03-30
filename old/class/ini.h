#pragma once
//A header file that is testing
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
namespace cppweb{
class IniConfig{
public:
	class Object{
	public:
		Object(const std::string& value):value(value),error(NULL){}
		inline std::string toString(){
			return value;
		}
		int toFloat(){
			float result=0;
			auto flag=sscanf(value.c_str(),"%f",&result);
			if(flag<=0){
				error=(value+std::string(" cannot be float")).c_str();
				return -1;
			}
			return result;
		}
		int toBool(){
			if(value=="true"){
				return true;
			}else if(value=="false"){
				return false;
			}else{
				error=(value+std::string(" cannot be bool")).c_str();
				return false;
			}
		}
		int toInt(){
			int result=0;
			auto flag=sscanf(value.c_str(),"%d",&result);
			if(flag<=0){
				error=(value+std::string(" cannot be int")).c_str();
				return -1;
			}
			return result;
		}
	private:
		std::string value;
		const char* error;
	};
	typedef std::unordered_map<std::string,std::unordered_map<std::string,Object>> KeyMap;
private:
	KeyMap root;
	std::string data;
	const char* error;
public:
	IniConfig(const std::string& data):data(data),error(NULL){
		deleteComment(this->data);
		std::vector<std::string> arr;
		stringSplit(arr,data,'\n');
		analyseText(arr);
	};
	inline const KeyMap& getAnalyseResult(){
		return root;
	}
	std::string createConfig(const KeyMap& root){
		std::string result;
		for(auto iter:root){
			result+="["+iter.first+"]\n";
			for(auto now:iter.second){
				result+=now.first+" = "+now.second.toString()+"\n";
			}
		}
		return result;
	}
	inline const char* lastError(){
		return error;
	}
public:
	bool analyseText(std::vector<std::string>& arr){
		std::string section;
		bool flag=true;
		for(auto& now:arr){
			if(now.size()==0){
				continue;
			}
			if(now[0]=='['){
				flag=findSection(section,now);
				if(!flag){
					return false;
				}else{
					root.insert({section,std::unordered_map<std::string,Object>()});
				}
			}else{
				if(section.size()==0){
					error="can not find begin section";
					return false;
				}
				flag=findKeyValue(now,root[section]);
				if(!flag){
					return false;
				}
			}
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
			}else if(ch=='\r'||ch==' '){
				continue;
			}
			if(!flag){
				temp+=ch;
			}
		}
		data=temp;
	}
private:
	bool findSection(std::string& result,const std::string data){
		result.clear();
		if(data.size()==0||data[0]!='['){
			error="section begin without [";
			return false;
		}
		if(data.find(']')==data.npos){
			error="section end without ]";
			return false;
		}
		auto pos=data.find(']');
		result=data.substr(1,pos-1);
		return true;
	}
	bool findKeyValue(const std::string& data,std::unordered_map<std::string,Object>& result){
		if(data.find('=')==data.npos){
			error="can not find =";
			return false;
		}
		auto pos=data.find('=');
		result.insert({data.substr(0,pos),data.substr(pos+1,data.size())});
		return true;
	}
	void stringSplit(std::vector<std::string>& elems,const std::string& str, char delim) {
		std::size_t previous = 0;
		std::size_t current = str.find(delim);
		while (current != std::string::npos) {
			if (current > previous) {
				elems.push_back(str.substr(previous, current - previous));
			}
			previous = current + 1;
			current = str.find(delim, previous);
		}
		if (previous != str.size()) {
			elems.push_back(str.substr(previous));
		}
	}
};
};
