//THIS FILE IS PART OF PROJECT CPPWEB
//
//THIS PROGRAM IS FREE SOFTWARE. IS LICENSED UNDER AGPL
//
//Copyright (c) 2022 chenxuan 
/* #define CPPWEB_OPENSSL */
#ifndef _CPPWEB_H_
#define _CPPWEB_H_

#ifndef _WIN32
#include<signal.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<unistd.h>
#include<netdb.h>
#else
#include<winsock2.h>
#endif

#include<string.h>
#include<pthread.h>
#include<stdarg.h>
#include<time.h>
#include<iostream>
#include<cstdlib>
#include<queue>
#include<vector>
#include<stack>
#include<string>
#include<functional>
#include<type_traits>
#include<unordered_map>
#include<initializer_list>

#ifdef CPPWEB_OPENSSL
#include<openssl/ssl.h>
#include<openssl/err.h>
#endif

namespace cppweb{
//this class for windows 
#ifdef _WIN32
#define socklen_t int
#define MSG_DONTWAIT 0
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-17 14:50:16
* description:class for windows socket init
* example: do not use the class
***********************************************/
class WSAinit{
public:
	WSAinit()
	{
		WSADATA wsa;//web server api data
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
		{
			printf("wsadata wrong\n");
			exit(0);
		}
	}
	~WSAinit()
	{
		WSACleanup();
	}
}_wsaInit;
#endif
/*******************************
 * author:chenxuan
 * class:class for analyse json and create json text 
 * example:more information about it is in https://gitee.com/chenxuan520/cppjson
******************************/
class Json{
public:
	enum TypeJson{//object type
		INT=0,FLOAT=1,ARRAY=2,OBJ=3,STRING=4,BOOL=5,STRUCT=6,EMPTY=7
	};
	struct Object{
		TypeJson type;
		TypeJson arrType;
		std::string key;
		std::string strVal;
		std::vector<Object*> arr;
		Object* objVal;
		Object* nextObj;
		double floVal;
		int intVal;
		bool boolVal;
		bool isData;
		Object()
		{
			floVal=0;
			intVal=0;
			boolVal=false;
			isData=false;
			nextObj=NULL;
			objVal=NULL;
		}
		Object* operator[](const char* key)
		{
			Object* now=this->nextObj;
			while(now!=NULL)
			{
				if(now->key==key)
					return now;
				now=now->nextObj;
			}
			return NULL;
		}
	};
private:
	struct InitType{//struct for ctreate such as {{"as","ds"}} in init;
		TypeJson type=STRING;
		void* pval=NULL;
		const char* error=NULL;
		unsigned len=0;
		~InitType()
		{
			if(pval)
			{
				free(pval);
				pval=NULL;
			}
		}
		InitType(const InitType& old)
		{
			this->type=old.type;
			this->error=old.error;
			if(old.pval!=NULL)
			{
				pval=malloc(sizeof(char)*old.len);
				if(pval==NULL)
				{
					error="malloc worng";
					type=EMPTY;
					return;
				}
				memcpy(pval,old.pval,old.len);
			}
		}
		InitType(std::initializer_list<std::pair<std::string,InitType>> listInit)
		{
			type=OBJ;
			std::string result="";
			Json json(listInit);
			result+=json();
			pval=malloc(sizeof(char)*(result.size()+10));
			len=sizeof(char)*(result.size()+10);
			memset(pval,0,sizeof(char)*(result.size()+10));
			if(pval==NULL)
			{
				error="malloc wrong";
				type=EMPTY;
				return;
			}
			strcpy((char*)pval,result.c_str());
		}
		template<typename T>
		InitType(std::initializer_list<T> listInit)
		{
			Json json;
			type=ARRAY;
			char* now=NULL;
			T* arr=new T[listInit.size()];
			int i=0;
			for(auto iter=listInit.begin();iter!=listInit.end();iter++)
			{
				arr[i]=*iter;
				i++;
			}
			int arrlen=listInit.size();
			if(std::is_same<T,int>::value)
				now=json.createArray(INT,arrlen,(void*)arr);
			else if(std::is_same<T,double>::value)
				now=json.createArray(FLOAT,arrlen,(void*)arr);
			else if(std::is_same<T,const char*>::value)
				now=json.createArray(STRING,arrlen,(void*)arr);
			else if(std::is_same<T,bool>::value)
				now=json.createArray(BOOL,arrlen,(void*)arr);
			else
			{
				error="type wrong";
				type=EMPTY;
			}
			pval=malloc(sizeof(char)*strlen(now)+10);
			len=sizeof(char)*strlen(now)+10;
			strcpy((char*)pval,now);
			delete [] arr;
		}
		template<typename T>
		InitType(T val)
		{
			if(std::is_same<T,int>::value)
				type=INT;
			else if(std::is_same<T,double>::value)
				type=FLOAT;
			else if(std::is_same<T,bool>::value)
				type=BOOL;
			else
				type=EMPTY;
			pval=malloc(sizeof(T));
			if(pval==NULL)
			{
				type=EMPTY;
				error="malloc wrong";
			}
			else
			{
				len=sizeof(T);
				memset(pval,0,sizeof(T));
				*(T*)pval=val;
			}
		}
		InitType(const char* pt)
		{
			if(pt==nullptr)
				type=EMPTY;
			else
			{
				type=STRING;
				pval=malloc(sizeof(char)*strlen(pt)+10);
				len=sizeof(sizeof(char)*strlen(pt)+10);
				if(pval==NULL)
				{
					type=EMPTY;
					error="malloc worng";
				}
				else
				{
					memset(pval,0,sizeof(char)*strlen(pt)+10);
					strcpy((char*)pval,pt);
				}
			}
		}
	};
private:
	char* text;
	const char* error;
	const char* nowKey;
	char* word;
	char* result;
	unsigned maxLen;
	unsigned floNum;
	unsigned defaultSize;
	Object* obj;
	std::unordered_map<char*,unsigned> memory;
	/* std::unordered_map<std::string,Object*> hashMap; */
	std::unordered_map<char*,char*> bracket;
public:
	Json()
	{
		error=NULL;
		obj=NULL;
		text=NULL;
		word=NULL;
		result=NULL;
		nowKey=NULL;
		maxLen=256;
		floNum=3;
		defaultSize=128;
		result=this->createObject();
		word=(char*)malloc(sizeof(char)*maxLen);
		if(word==NULL||result==NULL)
		{
			error="malloc wrong";
			return;
		}
		memset(word,0,sizeof(char)*maxLen);
	}
	Json(std::initializer_list<std::pair<std::string,InitType>> initList):Json()
	{
		for(auto iter=initList.begin();iter!=initList.end();iter++)
		{
			if(iter->second.pval==NULL&&iter->second.type!=EMPTY)
			{
				error="init wrong";
				return;
			}
			if(iter->second.error!=NULL)
			{
				error="init wrong";
				return;
			}
			switch(iter->second.type)
			{
			case STRING:
				if(false==addKeyVal(result,STRING,iter->first.c_str(),iter->second.pval))
					return;
				break;
			case INT:
				if(false==addKeyVal(result,INT,iter->first.c_str(),*(int*)iter->second.pval))
					return;
				break;
			case FLOAT:
				if(false==addKeyVal(result,FLOAT,iter->first.c_str(),*(double*)iter->second.pval))
					return;
				break;
			case BOOL:
				if(false==addKeyVal(result,BOOL,iter->first.c_str(),*(bool*)iter->second.pval))
					return;
				break;
			case EMPTY:
				if(false==addKeyVal(result,EMPTY,iter->first.c_str(),NULL))
					return;
				break;
			case OBJ:
				if(false==addKeyVal(result,OBJ,iter->first.c_str(),(char*)iter->second.pval))
					return;
				break;
			case ARRAY:
				if(false==addKeyVal(result,ARRAY,iter->first.c_str(),(char*)iter->second.pval))
					return;
				break;
			default:
				return;
			}
		}
	}
	Json(const char* jsonText):Json()
	{
		if(jsonText==NULL||strlen(jsonText)==0)
		{
			error="message error";
			return;
		}
		text=(char*)malloc(strlen(jsonText)+10);
		if(text==NULL)
		{
			error="malloc wrong";
			return;
		}
		memset(text,0,strlen(jsonText)+10);
		strcpy(text,jsonText);
		deleteComment();
		deleteSpace();
		if(false==pairBracket())
		{
			error="pair bracket wrong";
			return;
		}
		if(text[0]!='{')
		{
			error="text wrong";
			return;
		}
		obj=analyseObj(text,bracket[text]);
		if(obj==NULL)
		{
			error="malloc wrong";
			return;
		}
	}
	~Json()
	{
		deleteNode(obj);
		if(word!=NULL)
		{
			free(word);
			word=NULL;
		}
		if(text!=NULL)
		{
			free(text);
			result=NULL;
		}
		for(auto iter=memory.begin();iter!=memory.end();iter++)
			if(iter->first!=NULL)
				free(iter->first);
	}
	const char* formatPrint(const Object* exmaple)
	{
		char* buffer=(char*)malloc(sizeof(char)*defaultSize*10);
		if(buffer==NULL)
		{
			error="malloc wrong";
			return NULL;
		}
		memset(buffer,0,sizeof(char)*defaultSize*10);
		memory.insert(std::pair<char*,unsigned>{buffer,sizeof(char)*defaultSize*10});
		printObj(buffer,exmaple);
		return buffer;
	}
	template<typename T>
	bool addKeyVal(char*& obj,const char* key,T value)
	{
		if(std::is_same<T,int>::value)
			return addKeyVal(obj,INT,key,value);
		else if(std::is_same<T,double>::value)
			return addKeyVal(obj,FLOAT,key,value);
		else if(std::is_same<T,char*>::value)
			return addKeyVal(obj,OBJ,key,value);
		else if(std::is_same<T,const char*>::value)
			return addKeyVal(obj,STRING,key,value);
		else if(std::is_same<T,bool>::value)
			return addKeyVal(obj,BOOL,key,value);
		else 
			return addKeyVal(obj,EMPTY,key,NULL);
	}
	bool addKeyVal(char*& obj,TypeJson type,const char* key,...)
	{
		if(obj==NULL)
		{
			error="null buffer";
			return false;
		}
		if(key==NULL)
		{
			error="key null";
			return false;
		}
		va_list args;
		va_start(args,key);
		if(obj[strlen(obj)-1]=='}')
		{
			if(obj[strlen(obj)-2]!='{')
				obj[strlen(obj)-1]=',';
			else
				obj[strlen(obj)-1]=0;
		}
		if(memory.find(obj)==memory.end())
		{
			error="wrong object";
			return false;
		}
		while(memory[obj]-strlen(obj)<strlen(key)+4)
			obj=enlargeMemory(obj);
		sprintf(obj,"%s\"%s\":",obj,key);
		int valInt=0;
		char* valStr=NULL;
		double valFlo=9;
		bool valBool=false;
		switch(type)
		{
		case INT:
			valInt=va_arg(args,int);
			while(memory[obj]-strlen(obj)<15)
				obj=enlargeMemory(obj);
			sprintf(obj,"%s%d",obj,valInt);
			break;
		case FLOAT:
			valFlo=va_arg(args,double);
			while(memory[obj]-strlen(obj)<15)
				obj=enlargeMemory(obj);
			sprintf(obj,"%s%.*lf",obj,floNum,valFlo);
			break;
		case STRING:
			valStr=va_arg(args,char*);
			if(valStr==NULL)
			{
				error="null input";
				return false;
			}
			while(memory[obj]-strlen(obj)<strlen(obj)+5)
				obj=enlargeMemory(obj);
			sprintf(obj,"%s\"%s\"",obj,valStr);
			break;
		case EMPTY:
			while(memory[obj]-strlen(obj)<5)
				obj=enlargeMemory(obj);
			strcat(obj,"null");
			break;
		case BOOL:
			valBool=va_arg(args,int);
			while(memory[obj]-strlen(obj)<5)
				obj=enlargeMemory(obj);
			if(valBool==true)
				strcat(obj,"true");
			else
				strcat(obj,"false");
			break;
		case OBJ:
		case ARRAY:
			valStr=va_arg(args,char*);
			while(memory[obj]-strlen(obj)<strlen(obj)+5)
				obj=enlargeMemory(obj);
			if(valStr==NULL)
			{
				error="null input";
				return false;
			}
			sprintf(obj,"%s%s",obj,valStr);
			break;
		default:
			error="can not insert this type";
			strcat(obj,"}");
			return false;
		}
		strcat(obj,"}");
		return true;
	}
	char* createObject()
	{
		char* now=(char*)malloc(sizeof(char)*defaultSize);
		if(now==NULL)
		{
			error="init worng";
			return NULL;
		}
		else
			memory.insert(std::pair<char*,unsigned>{now,defaultSize});
		memset(now,0,sizeof(char)*defaultSize);
		strcpy(now,"{}");
		return now;
	}
	char* createArray(TypeJson type,unsigned arrLen,void* arr)
	{
		if(arr==NULL)
		{
			error="null input";
			return NULL;
		}
		char* now=(char*)malloc(sizeof(char)*defaultSize);
		if(now==NULL)
		{
			error="malloc worng";
			return NULL;
		}
		else
			memory.insert(std::pair<char*,unsigned>{now,defaultSize});
		memset(now,0,sizeof(char)*defaultSize);
		strcat(now,"[");
		int* arrInt=(int*)arr;
		double* arrFlo=(double*)arr;
		char** arrStr=(char**)arr;
		bool* arrBool=(bool*)arr;
		unsigned i=0;
		switch(type)
		{
		case INT:
			for(i=0;i<arrLen;i++)
			{
				while(memory[now]-strlen(now)<std::to_string(arrInt[i]).size()+3)
					now=enlargeMemory(now);
				sprintf(now,"%s%d,",now,arrInt[i]);
			}
			break;
		case FLOAT:
			for(i=0;i<arrLen;i++)
			{
				while(memory[now]-strlen(now)<std::to_string(arrInt[i]).size()+3)
					now=enlargeMemory(now);
				sprintf(now,"%s%.*lf,",now,floNum,arrFlo[i]);
			}
			break;
		case STRING:
			for(i=0;i<arrLen;i++)
			{
				while(memory[now]-strlen(now)<strlen(arrStr[i])+5)
					now=enlargeMemory(now);
				sprintf(now,"%s\"%s\",",now,arrStr[i]);
			}
			break;
		case OBJ:
		case ARRAY:
			for(i=0;i<arrLen;i++)
			{
				while(memory[now]-strlen(now)<strlen(arrStr[i])+4)
					now=enlargeMemory(now);
				sprintf(now,"%s%s,",now,arrStr[i]);
			}
			break;
		case BOOL:
			for(i=0;i<arrLen;i++)
			{
				while(memory[now]-strlen(now)<6)
					now=enlargeMemory(now);
				if(arrBool[i])
					strcat(now,"true,");
				else
					strcat(now,"false,");
			}
			break;
		default:
			error="struct cannot be a array";
			break;
		}
		if(now[strlen(now)-1]==',')
			now[strlen(now)-1]=']';
		else
			strcat(now,"]");
		now[strlen(now)]=0;
		return now;
	}
	Object* operator[](const char* key)
	{
		return (*obj)[key];
		/* if(hashMap.find(std::string(key))==hashMap.end()) */
		/* 	return NULL; */
		/* return hashMap.find(std::string(key))->second; */
	}
	char*& operator()()
	{
		return result;
	}
	Json& operator()(const char* key)
	{
		nowKey=key;
		return *this;
	}
	template<typename T>
	Json& operator=(T value)
	{
		this->addKeyVal(this->result,nowKey,value);
		return *this;
	}
	inline Object* getRootObj()
	{
		return obj;
	}
	inline const char* lastError()
	{
		return error;
	}
	inline void changeSetting(unsigned defaultSize=128,unsigned keyValMaxLen=256,unsigned floNum=3)
	{
		this->defaultSize=defaultSize;
		this->maxLen=keyValMaxLen>maxLen?keyValMaxLen:maxLen;
		this->floNum=floNum;
	}
private:
	char* enlargeMemory(char* old)
	{
		if(memory.find(old)==memory.end())
			return old;
		unsigned temp=memory[old];
		temp*=2;
		void* strTemp=realloc(old,temp);
		if(strTemp==NULL)
			return old;
		memory.erase(memory.find(old));
		memory.insert(std::pair<char*,unsigned>{(char*)strTemp,temp});
		return (char*)strTemp;
	}
	Object* analyseObj(char* begin,char* end)
	{
		Object * root=new Object,*last=root;
		root->type=STRUCT;
		char* now=begin+1,*next=now;
		char temp=*end;
		if(word==NULL)
		{
			error="malloc wrong";
			return NULL;
		}
		memset(word,0,sizeof(char)*maxLen);
		*end=0;
		while(now<end)
		{
			Object* nextObj=new Object;
			memset(word,0,sizeof(char)*maxLen);
			findString(now,word,maxLen);
			nextObj->key=word;
			/* hashMap.insert(std::pair<std::string,Object*>{word,nextObj}); */
			now+=strlen(word)+3;
			memset(word,0,sizeof(char)*maxLen);
			if(*now=='\"')
			{
				nextObj->type=STRING;
				next=strchr(now+1,'\"');
				while(next!=NULL&&*(next-1)=='\\')
					next=strchr(next+1,'\"');
				if(next==NULL)
				{
					error="string wrong";
					return NULL;
				}
				if(next-now+3>maxLen)
				{
					char* tempStr=(char*)realloc(word,next-now+3);
					if(tempStr!=NULL)
					{
						word=tempStr;
						maxLen=next-now+3;
					}
				}
				for(unsigned i=0;now+i+1<next&&i<maxLen;i++)
					word[i]=*(now+i+1);
				word[strlen(word)]=0;
				nextObj->strVal=word;
				now=next+1;
				if(*now==',')
					now++;
			}
			else if(('0'<=*now&&'9'>=*now)||*now=='-')
			{
				next=now;
				nextObj->type=INT;
				while(*next!=','&&*next!=0)
				{
					next++;
					if(*next=='.')
						nextObj->type=FLOAT;
				}
				if(nextObj->type==INT)
					sscanf(now,"%d",&nextObj->intVal);
				else
					sscanf(now,"%lf",&nextObj->floVal);
				now=next+1;
			}
			else if(*now=='[')
			{
				next=bracket[now];
				if(next==NULL)
				{
					error="format wrong";
					return root;
				}
				nextObj->type=ARRAY;
				nextObj->arrType=analyseArray(now,next,nextObj->arr);
				now=next+1;
				if(*now==',')
					now++;
			}
			else if(*now=='{')
			{
				next=bracket[now];
				if(next==NULL)
				{
					error="format wrong";
					return root;
				}
				nextObj->type=OBJ;
				nextObj->objVal=analyseObj(now,next);
				now=next+1;
				if(*now==',')
					now++;
			}
			else if(strncmp(now,"true",4)==0)
			{
				nextObj->type=BOOL;
				now+=4;
				if(*now==',')
					now++;
				nextObj->boolVal=true;
			}
			else if(strncmp(now,"false",5)==0)
			{
				nextObj->type=BOOL;
				now+=5;
				if(*now==',')
					now++;
				nextObj->boolVal=false;
			}
			else if(strncmp(now,"null",4)==0)
			{
				nextObj->type=EMPTY;
				now+=4;
				if(*now==',')
					now++;
			}
			else
			{
				error="text wrong";
				return root;
			}
			last->nextObj=nextObj;
			last=nextObj;
		}
		*end=temp;
		return root;
	}
	TypeJson analyseArray(char* begin,char* end,std::vector<Object*>& arr)
	{
		char* now=begin+1,*next=end;
		if(word==NULL)
		{
			error="malloc wrong";
			return INT;
		}
		memset(word,0,sizeof(char)*maxLen);
		Object* nextObj=NULL;
		if((*now>='0'&&*now<='9')||*now=='-')
		{
			next=now;
			while(next<end&&*next!=',')
				next++;
			TypeJson type=judgeNum(now,next);
			while(now<end&&now!=NULL)
			{
				nextObj=new Object;
				nextObj->isData=true;
				nextObj->type=type;
				if(nextObj->type==INT)
				{
					findNum(now,type,&nextObj->intVal);
					arr.push_back(nextObj);
				}
				else
				{
					findNum(now,type,&nextObj->floVal);
					arr.push_back(nextObj);
				}
				now=strchr(now+1,',');
				if(now!=NULL)
					now++;
			}
			nextObj->arrType=type;
		}
		else if(*now=='\"')
		{
			while(now<end&&now!=NULL)
			{
				findString(now,word,maxLen);
				nextObj=new Object;
				nextObj->type=STRING;
				nextObj->isData=true;
				nextObj->strVal=word;
				arr.push_back(nextObj);
				now=strchr(now+1,',');
				if(now==NULL)
					break;
				now+=1;
			}
		}
		else if(strncmp(now,"true",4)==0||strncmp(now,"false",5)==0)
		{
			while(now<end&&now!=NULL)
			{
				nextObj=new Object;
				nextObj->type=BOOL;
				nextObj->isData=true;
				nextObj->boolVal=strncmp(now,"true",4)==0;
				arr.push_back(nextObj);
				now=strchr(now+1,',');
				if(now==NULL)
					break;
				now+=1;
			}
		}
		else if(*now=='{')
		{
			while(now<end&&now!=NULL)
			{
				next=bracket[now];
				nextObj=analyseObj(now,next);
				nextObj->type=OBJ;
				nextObj->isData=true;
				arr.push_back(nextObj);
				now=next;
				now=strchr(now+1,',');
				if(now==NULL)
					break;
				now+=1;
			}
		}
		else if(*now=='[')
		{
			while(now<end&&now!=NULL)
			{
				next=bracket[now];
				nextObj=new Object;
				TypeJson type=analyseArray(now,next,nextObj->arr);
				nextObj->type=ARRAY;
				nextObj->arrType=type;
				nextObj->isData=true;
				arr.push_back(nextObj);
				now=next;
				now=strchr(now+1,',');
				if(now==NULL)
					break;
				now+=1;
			}
		}
		else if(*now==']')
			return INT;
		else
		{
			error="array find wrong";
			return INT;
		}
		return nextObj->type;
	}
	void findString(const char* begin,char*& buffer,unsigned& buffLen)
	{
		const char* now=begin+1,*nextOne=now;
		nextOne=strchr(now+1,'\"');
		while(nextOne!=NULL&&*(nextOne-1)=='\\')
			nextOne=strchr(nextOne+1,'\"');
		if(nextOne==NULL)
		{
			error="text wrong";
			return;
		}
		if(buffLen<nextOne-now)
		{
			char* temp=(char*)realloc(buffer,sizeof(char)*(nextOne-now+10));
			if(temp!=NULL)
			{
				buffer=temp;
				buffLen=nextOne-now+10;
			}
		}
		for(unsigned i=0;now+i<nextOne&&i<buffLen-1;i++)
			buffer[i]=*(now+i);
		buffer[strlen(buffer)]=0;
	}
	void findNum(const char* begin,TypeJson type,void* pnum)
	{
		if(type==INT)
		{
			if(sscanf(begin,"%d",(int*)pnum)<1)
				error="num wrong";
		}
		else
		{
			if(sscanf(begin,"%lf",(double*)pnum)<1)
				error="num wrong";
		}
	}
	inline TypeJson judgeNum(const char* begin,const char* end)
	{
		for(unsigned i=0;i+begin<end;i++)
			if(begin[i]=='.')
				return FLOAT;
		return INT;
	}
	void deleteComment()
	{
		unsigned flag=0;
		for(unsigned i=0;i<strlen(text);i++)
		{
			if(text[i]=='\"'&&text[i-1]!='\\')
				flag++;
			else if(flag%2==0&&text[i]=='/'&&i+1<strlen(text)&&text[i+1]=='/')
				while(text[i]!='\n'&&i<strlen(text))
				{
					text[i]=' ';
					i++;
				}
			else if(flag%2==0&&text[i]=='/'&&i+1<strlen(text)&&text[i+1]=='*')
				while(i+1<strlen(text))
				{
					if(text[i+1]=='/'&&text[i]=='*')
					{
						text[i]=' ';
						text[i+1]=' ';
						break;
					}
					text[i]=' ';
					i++;
				}
			else 
				continue;
		}
	}
	void deleteSpace()
	{
		unsigned j=0,k=0;
		unsigned flag=0;
		for(j=0,k=0; text[j]!='\0'; j++)
		{
			if(text[j]!='\r'&&text[j]!='\n'&&text[j]!='\t'&&(text[j]!=' '||flag%2!=0))
				text[k++]=text[j];
			if(text[j]=='\"'&&j>0&&text[j-1]!='\\')
				flag++;
		}
		text[k]=0;
	}
	void deleteNode(Object* root)
	{
		if(root==NULL)
			return;
		if(root->nextObj!=NULL)
			deleteNode(root->nextObj);
		if(root->arr.size()>0)
			for(unsigned i=0;i<root->arr.size();i++)
				deleteNode(root->arr[i]);
		if(root->objVal!=NULL)
			deleteNode(root->objVal);
		delete root;
		root=NULL;
	}
	bool pairBracket()
	{
		unsigned flag=0;
		std::stack<char*> sta;
		for(unsigned i=0;i<strlen(text);i++)
		{
			if((text[i]=='['||text[i]=='{')&&flag%2==0)
				sta.push(text+i);
			else if((text[i]==']'||text[i]=='}')&&flag%2==0)
			{
				if(sta.empty())
					return false;
				if(text[i]==']'&&*sta.top()!='[')
					return false;
				if(text[i]=='}'&&*sta.top()!='{')
					return false;
				bracket.insert(std::pair<char*,char*>{sta.top(),&text[i]});
				sta.pop();
			}
			else if(text[i]=='\"'&&i>0&&text[i-1]!='\\')
				flag++;
			else
				continue;
		}
		if(!sta.empty())
			return false;
		return true;
	}
	bool printObj(char*& buffer,const Object* obj)
	{
		unsigned deep=0;
		char* line=strrchr(buffer,'\n');
		if(line==NULL)
			deep=1;
		else
			deep=buffer+strlen(buffer)-line;
		strcat(buffer,"{\n");
		Object* now=obj->nextObj;
		while(now!=NULL)
		{
			while(memory[buffer]-strlen(buffer)<now->key.size()+now->strVal.size()+20+deep*5)
				buffer=enlargeMemory(buffer);
			for(unsigned i=0;i<deep+4;i++)
				strcat(buffer," ");
			switch(now->type)
			{
			case INT:
				sprintf(buffer,"%s\"%s\":%d,",buffer,now->key.c_str(),now->intVal);
				break;
			case FLOAT:
				sprintf(buffer,"%s\"%s\":%.*lf,",buffer,now->key.c_str(),floNum,now->floVal);
				break;
			case STRING:
				sprintf(buffer,"%s\"%s\":\"%s\",",buffer,now->key.c_str(),now->strVal.c_str());
				break;
			case BOOL:
				if(now->boolVal)
					sprintf(buffer,"%s\"%s\":true,",buffer,now->key.c_str());
				else
					sprintf(buffer,"%s\"%s\":false,",buffer,now->key.c_str());
				break;
			case OBJ:
				sprintf(buffer,"%s\"%s\":",buffer,now->key.c_str());
				printObj(buffer,now->objVal);
				strcat(buffer,",");
				break;
			case ARRAY:
				sprintf(buffer,"%s\"%s\":",buffer,now->key.c_str());
				printArr(buffer,now->arrType,now->arr);
				strcat(buffer,",");
				break;
			case EMPTY:
				sprintf(buffer,"%s\"%s\":null",buffer,now->key.c_str());
				break;
			default:
				error="struct cannot print";
				return false;
			}
			strcat(buffer,"\n");
			now=now->nextObj;
			if(now==NULL)
				*strrchr(buffer,',')=' ';
		}
		for(unsigned i=0;i<deep-1;i++)
			strcat(buffer," ");
		strcat(buffer,"}");
		return true;
	}
	bool printArr(char*& buffer,TypeJson type,const std::vector<Object*>& arr)
	{
		unsigned deep=0;
		char* line=strrchr(buffer,'\n');
		if(line==NULL)
			deep=0;
		else
			deep=buffer+strlen(buffer)-line;
		strcat(buffer,"[\n");
		for(unsigned i=0;i<arr.size();i++)
		{
			for(unsigned i=0;i<deep+4;i++)
				strcat(buffer," ");
			while(memory[buffer]-strlen(buffer)<arr[i]->strVal.size()+20+deep*5)
				buffer=enlargeMemory(buffer);
			switch(type)
			{
			case INT:
				sprintf(buffer,"%s%d,",buffer,arr[i]->intVal);
				break;
			case FLOAT:
				sprintf(buffer,"%s%.*lf,",buffer,floNum,arr[i]->floVal);
				break;
			case STRING:
				sprintf(buffer,"%s\"%s\",",buffer,arr[i]->strVal.c_str());
				break;
			case BOOL:
				if(arr[i]->boolVal)
					strcat(buffer,"true,");
				else
					strcat(buffer,"false,");
				break;
			case OBJ:
				printObj(buffer,arr[i]);
				strcat(buffer,",");
				break;
			case ARRAY:
				printArr(buffer,arr[i]->arrType,arr[i]->arr);
				strcat(buffer,",");
				break;
			default:
				error="struct cannot print";
				return false;
			}
			strcat(buffer,"\n");
			if(i==arr.size()-1)
				*strrchr(buffer,',')=' ';
		}
		for(unsigned i=0;i<deep-1;i++)
			strcat(buffer," ");
		strcat(buffer,"]");
		return true;
	}
};
/*******************************
 * author:chenxuan
 * class:class for run server in background and use guard
 * example:the same as its name,is only for linux
******************************/
class ProcessCtrl{
public:
	static int childPid;
	static int backGround()
	{
		int pid=0;
#ifndef _WIN32
		if((pid=fork())!=0)
		{
			printf("process pid=%d\n",pid);
			exit(0);
		}
#endif
		return pid;
	}
	static void guard()
	{
#ifndef _WIN32
		signal(SIGINT,endGuard);
		signal(SIGQUIT,endGuard);
		signal(SIGTERM,endGuard);
		while(1)
		{
			int pid=fork();
			if(pid!=0)
			{
				childPid=pid;
				waitpid(pid, NULL, 0);
				sleep(15);
			}
			else
				break;
		}
#endif
	}
private:
#ifndef _WIN32
	static void endGuard(int)
	{
		if(ProcessCtrl::childPid!=0)
			kill(ProcessCtrl::childPid,2);
		exit(0);
	}
#endif
};
int ProcessCtrl::childPid=0;
/*******************************
 * author:chenxuan
 * class:class for trie and get route faster 
 * example:{
 * Trie<type> tree;
 * tree.insert(str,type);
 * }
******************************/
template<class T>
class Trie {
private:
	struct Node{
		Node* next[77];
		bool stop;
		T* data;
		Node()
		{
			for(unsigned i=0;i<77;i++)
				next[i]=NULL;
			stop=false;
		}
	};
	Node* root;
	void cleanMemory(Node* root)
	{
		for(unsigned i=0;i<77;i++)
			if(root->next[i]!=NULL)
				cleanMemory(root->next[i]);
		delete root;
	}
public:
	Trie() {
		root=new Node;
	}
	~Trie(){
		if(root!=NULL)
			cleanMemory(root);
	}
	bool insert(const char* word,T* data) {
		Node* temp=root;
		for(unsigned i=0;word[i]!=0;i++)
		{
			if(word[i]-46<0||word[i]-46>76)
				return false;
			if(temp->next[word[i]-46]!=NULL)
				temp=temp->next[word[i]-46];
			else
			{
				temp->next[word[i]-46]=new Node;
				temp=temp->next[word[i]-46];
			}
		}
		temp->stop=true;
		temp->data=data;
		return true;
	}
	T* search(const char* word,std::function<bool(const T*,bool isLast)> func) {
		Node* temp=root;
		for(unsigned i=0;word[i]!=0;i++)
		{
			if(word[i]-46<0||word[i]-46>76)
				return NULL;
			if(temp->next[word[i]-46]==NULL)
				return NULL;
			else
			{
				temp=temp->next[word[i]-46];
				if(temp->stop&&func(temp->data,word[i+1]==0))
					return temp->data;
			}
		}
		return NULL;
	}
	bool check(const char* word)
	{
		Node* temp=root;
		for(unsigned i=0;word[i]!=0;i++)
		{
			if(word[i]-46<0||word[i]-46>76)
				return false;
			if(temp->next[word[i]-46]==NULL)
				return false;
			else
				temp=temp->next[word[i]-46];
		}
		if(temp->stop==true)
			return false;
		return true;
	}
};
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-17 14:48:50
* description:class for server base
* example:{
* ServerTcpIp server(port);
* server.bound();
* server.setlisten();
* }
***********************************************/
class ServerTcpIp{
public:
	enum Thing{
		CPPOUT=0,CPPIN=1,CPPSAY=2,
	};
protected:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now;
	int fd_count;//sum of clients in fd_set
	int epfd;//file descriptor to ctrl epoll
	char* hostip;//host IP 
	char* hostname;//host name
	const char* error;//error hapen
	int sock;//file descriptor of host;
	int sockC;//file descriptor to sign every client;
	sockaddr_in addr;//IPv4 of host;
	sockaddr_in client;//IPv4 of client;
	fd_set  fdClients;//file descriptor
#ifndef _WIN32
	epoll_event nowEvent;//a temp event to get event
	epoll_event* pevent;//all the event
#endif
#ifdef CPPWEB_OPENSSL
	SSL_CTX* ctx;//openssl to https struct 
	std::unordered_map<int,SSL*> sslHash;//hash to find sock and SSL*
#endif
protected:
	int* pfdn;//pointer if file descriptor
	int fdNumNow;//num of fd now
	int fdMax;//fd max num
public:
	bool addFd(int addsoc)//add file des criptor
	{
		bool flag=false;
		for(int i=0;i<fdNumNow;i++)
		{
			if(pfdn[i]==0)
			{
				pfdn[i]=addsoc;
				flag=true;//has free room
				break;
			}
		}
		if(flag==false)//no free room
		{
			if(fdNumNow>=fdMax)
			{
				pfdn=(int*)realloc(pfdn,sizeof(int)*(fdMax+32));//try to realloc
				if(pfdn==NULL)
					return false;
				fdMax+=31;
			}
			pfdn[fdNumNow]=addsoc;
			fdNumNow++;
		}
		return true;
	}
	bool deleteFd(int clisoc)//delete
	{
		for(int i=0;i<fdNumNow;i++)
		{
			if(pfdn[i]==clisoc)
			{
				pfdn[i]=0;
				return true;
			}
		}
		return false;
	}
	bool getFd(int* array,int* pcount,int arrayLen)//get epoll array
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow&&i<arrayLen;i++)
			array[i]=pfdn[i];
		return true;
	}
	bool findFd(int cliSoc)//find if socket is connect
	{
		for(int i=0;i<fdNumNow;i++)
			if(pfdn[i]==cliSoc)
				return true;
		return false;
	}
public:
	ServerTcpIp(unsigned short port=5200,int epollNum=1,int wait=5)
	{//port is bound ,epollNum is if open epoll model,wait is listen socket max wait
		sock=socket(AF_INET,SOCK_STREAM,0);//AF=addr family internet
		addr.sin_addr.s_addr=htonl(INADDR_ANY);//inaddr_any
		addr.sin_family=AF_INET;//af_intt IPv4
		addr.sin_port=htons(port);//host to net short
		fd_count=0;// select model
		sizeAddr=sizeof(sockaddr);
		backwait=wait;
		numClient=0;
		error=NULL;
		hostip=(char*)malloc(sizeof(char)*200);
		if(hostip==NULL)
			error="hostip worng";
		else
			memset(hostip,0,sizeof(char)*200);
		hostname=(char*)malloc(sizeof(char)*300);
		if(hostname==NULL)
			error="hostname wrong";
		else
			memset(hostname,0,sizeof(char)*300);
		FD_ZERO(&fdClients);//clean fdClients;
#ifndef _WIN32
		epfd=epoll_create(epollNum);
		if((pevent=(epoll_event*)malloc(512*sizeof(epoll_event)))==NULL)
			error="event wrong";
		else
			memset(pevent,0,sizeof(epoll_event)*512);
		memset(&nowEvent,0,sizeof(epoll_event));
		pfdn=(int*)malloc(sizeof(int)*64);
		if(pfdn==NULL)
			error="pfdn wrong";
		else
			memset(pfdn,0,sizeof(int)*64);
#endif
		fdNumNow=0;
		fdMax=64;
#ifdef CPPWEB_OPENSSL
#if OPENSSL_VERSION_NUMBER < 0x1010001fL
		SSL_load_error_strings();
		SSL_library_init();
#else
		OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
#endif
		ctx=SSL_CTX_new(TLS_method());
		if(ctx==NULL)
		{
			error="create ctx wrong";
			return;
		}
		SSL_CTX_set_options(ctx,
							SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
							SSL_OP_NO_COMPRESSION |
							SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
#endif
	}
	virtual ~ServerTcpIp()//clean server
	{
#ifndef _WIN32
		close(sock);
		close(sockC);
		close(epfd);
		if(pevent!=NULL)
			free(pevent);
#else
		closesocket(sock);
		closesocket(sockC);
		closesocket(epfd);
#endif
		if(pfdn!=NULL)
			free(pfdn);
		if(hostip!=NULL)
			free(hostip);
		if(hostname!=NULL)
			free(hostname);
#ifdef CPPWEB_OPENSSL
		if(ctx!=NULL)
			SSL_CTX_free(ctx);
#endif
	}
#ifdef CPPWEB_OPENSSL
	bool loadCertificate(const char* certPath,const char* keyPath,const char* passwd=NULL)
	{
		if(ctx==NULL)
			return false;
		if(SSL_CTX_use_certificate_chain_file(ctx,certPath)!=1)
		{
			error="cert load wrong";
			return false;
		}
		if(passwd!=NULL)
			SSL_CTX_set_default_passwd_cb_userdata(ctx,(void*)passwd);
		if(SSL_CTX_use_PrivateKey_file(ctx,keyPath,SSL_FILETYPE_PEM)!=1)
		{
			error="key load wrong";
			return false;
		}
		return true;
	}
	int acceptSocketSSL(sockaddr_in& newaddr)
	{
		int cli=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
		SSL* now=SSL_new(ctx);
		if(now==NULL)
		{
			error="ssl new wrong";
			return cli;
		}
		SSL_set_fd(now,cli);
		SSL_accept(now);
		sslHash.insert(std::pair<int,SSL*>{cli,now});
		return cli;
	}
	inline int receiveSocketSSL(int num,void* pget,int len,int=0)
	{
		SSL* ssl=NULL;
		if(sslHash.find(num)!=sslHash.end())
			ssl=sslHash[num];
		if(ssl!=NULL)
			return SSL_read(ssl,pget,len);
		else
			return recv(num,pget,len,0);
	}
	inline int sendSocketSSL(int num,const void* pget,int len,int=0)
	{
		SSL* ssl=NULL;
		if(sslHash.find(num)!=sslHash.end())
			ssl=sslHash[num];
		if(ssl!=NULL)
			return SSL_write(ssl,pget,len);
		else
			return send(num,pget,len,0);
	}
	inline int closeSSL(int cli)
	{
		if(sslHash.find(cli)!=sslHash.end())
		{
			SSL_shutdown(sslHash[cli]);
			SSL_free(sslHash[cli]);
			sslHash.erase(sslHash.find(cli));
		}
#ifndef _WIN32
		return close(cli);
#else
		return closesocket(cli);
#endif
	}
#endif
	inline bool bondhost()//bond myself first
	{
		if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
			return false;
		return true;
	}
	inline void setPort(unsigned short port)//change the port bound
	{
		addr.sin_port=htons(port);
	}
	bool setlisten()//set listem to accept second
	{
		if(listen(sock,backwait)==-1)
			return false;
		FD_SET(sock,&fdClients);
#ifndef _WIN32
		nowEvent.events=EPOLLIN;
		nowEvent.data.fd=sock;
		epoll_ctl(epfd,EPOLL_CTL_ADD,sock,&nowEvent);
#endif
		fd_count=sock;
		return true;
	}
	inline int acceptSocket(sockaddr_in& newaddr)
	{
#ifndef CPPWEB_OPENSSL
		return accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
#else
		return acceptSocketSSL(newaddr);
#endif
	}
	int acceptOne()//wait until success model one
	{
		sockC=accept(sock,(sockaddr*)&client,(socklen_t*)&sizeAddr);
		return sockC;
	}
	inline int receiveOne(void* pget,int len)//model one
	{
		return recv(sockC,(char*)pget,len,0);
	}
	inline int receiveSocket(int clisoc,void* pget,int len,int flag=0)
	{
#ifndef CPPWEB_OPENSSL
		return recv(clisoc,(char*)pget,len,flag);
#else
		return this->receiveSocketSSL(clisoc,pget,len,flag);
#endif
	}
	int receiveSocket(int cliSoc,std::string& buffer,int flag=0)
	{
		char temp[1024]={0};
		buffer.clear();
		int len=receiveSocket(cliSoc,temp,1024,flag);
		if(len<=0)
			return len;
		while(len==1024)
		{
			buffer.append(temp,1024);
			len=receiveSocket(cliSoc,temp,1024,flag);
		}
		return buffer.size();
	}
	inline int sendOne(const void* psen,int len)//model one
	{
		return send(sockC,(char*)psen,len,0);
	}
	inline int sendSocket(int socCli,const void* psen,int len,int flag=0)//send by socket
	{
#ifndef CPPWEB_OPENSSL
		return send(socCli,(char*)psen,len,flag);
#else
		return sendSocketSSL(socCli,psen,len,flag);
#endif
	}
	inline int closeSocket(int socCli)
	{
#ifndef CPPWEB_OPENSSL
#ifndef _WIN32
		return close(socCli);
#else
		return closesocket(socCli);
#endif
#else
		return closeSSL(socCli);
#endif
	}
	inline const char* lastError()
	{
		return this->error;
	}
	void selectModel(int (*pfunc)(Thing,int,ServerTcpIp&,void*),void* argv)
	{
		fd_set temp=fdClients;
#ifndef _WIN32
		int sign=select(fd_count+1,&temp,NULL,NULL,NULL);
		if(sign>0)
			for(int i=3;i<fd_count+1;i++)
				if(FD_ISSET(i,&temp))
				{
					if(i==sock)
					{
						if(fd_count<1024)
						{
							sockaddr_in newaddr={0,0,{0},{0}};
							int newClient=acceptSocket(newaddr);
							FD_SET(newClient,&fdClients);
							if(newClient>fd_count)
								fd_count=newClient;
							if(pfunc!=NULL)
							{
								int numGet=pfunc(CPPIN,newClient,*this,argv);
								if(numGet!=0)
								{
									FD_CLR(numGet,&fdClients);
									if(numGet==fd_count)
										for(int j=numGet-1;j>=0;j--)
											if(FD_ISSET(j,&fdClients))
											{
												fd_count=j;
												break;
											}
									closeSocket(newClient);
								}
							}
						}
						else
							continue;
					}
					else
					{
						if(pfunc!=NULL)
						{
							int numGet=pfunc(CPPSAY,i,*this,argv);
							if(numGet!=0)
							{
								FD_CLR(i,&fdClients);
								if(i==fd_count)
									for(int j=i-1;j>=0;j--)
										if(FD_ISSET(j,&fdClients))
										{
											fd_count=j;
											break;
										}
								closeSocket(i);
							}
						}
					}
				}
#else
		int sign=select(0,&temp,NULL,NULL,NULL);
		if(sign>0)
		{
			for(int i=0;i<(int)fdClients.fd_count;i++)
			{
				if(FD_ISSET(fdClients.fd_array[i],&temp))
				{
					if(fdClients.fd_array[i]==sock)
					{
						if(fdClients.fd_count<FD_SETSIZE)
						{
							SOCKADDR_IN newaddr={0,0,{0,0,0,0},0};
							SOCKET newClient=acceptSocket(newaddr);
							FD_SET(newClient,&fdClients);
							if(pfunc!=NULL)
							{
								int numGet=pfunc(CPPIN,newClient,*this,argv);
								if(numGet!=0)
								{
									FD_CLR(numGet,&fdClients);
									closeSocket(newClient);
								}
							}
						}
						else
							continue;
					}
					else
					{
						if(pfunc!=NULL)
						{
							int numGet=pfunc(CPPSAY,fdClients.fd_array[i],*this,argv);
							if(numGet!=0)
							{
								FD_CLR(numGet,&fdClients);
								closeSocket(numGet);
							}
						}
					}
				}
			}
		}
#endif
	}
#ifndef _WIN32
	bool selectModel(void* pget,int len,void* pneed,int (*pfunc)(Thing ,int ,int,void* ,void*,ServerTcpIp& ))
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
		fd_set temp=fdClients;
		Thing pthing=CPPOUT;
		int sign=select(fd_count+1,&temp,NULL,NULL,NULL);
		if(sign>0)
		{
			for(int i=3;i<fd_count+1;i++)
			{
				if(FD_ISSET(i,&temp))
				{
					if(i==sock)
					{
						if(fd_count<1024)
						{
							sockaddr_in newaddr={0,0,{0},{0}};
							int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
							FD_SET(newClient,&fdClients);
							if(newClient>fd_count)
								fd_count=newClient;
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(pfunc!=NULL)
							{
								if(pfunc(CPPIN,newClient,0,pget,pneed,*this))
									return false;
							}
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(i,(char*)pget,len,0);
						int socRec=i;
						if(sRec>0)
						{
							pthing=CPPSAY;
						}
						if(sRec<=0)
						{
							close(i);
							FD_CLR(i,&fdClients);
							if(i==fd_count)
								for(int j=i-1;j>=0;j--)
									if(FD_ISSET(j,&fdClients))
									{
										fd_count=j;
										break;
									}
							*(char*) pget=0;
							pthing=CPPOUT;
						}
						if(pfunc!=NULL)
						{
							if(pfunc(pthing,socRec,sRec,pget,pneed,*this))
								return false;
						}
					}
				}
			}
		}
		else
			return false;
		return true;
	}
#endif
	char* getHostName()//get self name
	{
		char name[300]={0};
		gethostname(name,300);
		memcpy(hostname,name,300);
		return hostname;
	}
	char* getHostIp()//get self ip
	{
		char name[300]={0};
		gethostname(name,300);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.s_addr,p,phost->h_length);
		memset(hostip,0,sizeof(char)*200);
		if(strlen(inet_ntoa(addr))>=200)
			return NULL;
		memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return hostip;
	}
	static const char* getPeerIp(int cliSoc,int* pcliPort)//get ip and port by socket
	{
		sockaddr_in cliAddr={0,0,{0},{0}};
		int len=sizeof(cliAddr);
		if(-1==getpeername(cliSoc,(sockaddr*)&cliAddr,(socklen_t*)&len))
			return NULL;
		*pcliPort=cliAddr.sin_port;
		return inet_ntoa(cliAddr.sin_addr); 
	}
	void epollModel(int (*pfunc)(Thing,int,ServerTcpIp&,void*),void* argv)
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
#ifndef _WIN32
		int eventNum=epoll_wait(epfd,pevent,512,-1),numGet=0;
		for(int i=0;i<eventNum;i++)
		{
			epoll_event temp=pevent[i];
			if(temp.data.fd==sock)
			{
				sockaddr_in newaddr={0,0,{0},{0}};
				int newClient=acceptSocket(newaddr);
				nowEvent.data.fd=newClient;
				nowEvent.events=EPOLLIN;
				epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
				if(pfunc!=NULL)
				{
					numGet=pfunc(CPPIN,newClient,*this,argv);
					if(numGet!=0)
					{
						epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
						closeSocket(temp.data.fd);
					}
				}
			}
			else
			{
				if(pfunc!=NULL)
				{
					numGet=pfunc(CPPSAY,temp.data.fd,*this,argv);
					if(numGet!=0)
					{
						epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
						closeSocket(temp.data.fd);
					}
				}
			}
		}
#else
		selectModel(pfunc,argv);
#endif
	}
#ifndef _WIN32
	bool epollModel(void* pget,int len,void* pneed,int (*pfunc)(Thing,int ,int ,void* ,void*,ServerTcpIp& ))
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
		Thing thing=CPPSAY;
		int eventNum=epoll_wait(epfd,pevent,512,-1);
		for(int i=0;i<eventNum;i++)
		{
			epoll_event temp=pevent[i];
			if(temp.data.fd==sock)
			{
				sockaddr_in newaddr={0,0,{0},{0}};
				int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
				nowEvent.data.fd=newClient;
				nowEvent.events=EPOLLIN;
				epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
				strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
				if(pfunc!=NULL)
				{
					if(pfunc(CPPIN,newClient,0,pget,pneed,*this))
						return false;
				}
			}
			else
			{
				int getNum=recv(temp.data.fd,(char*)pget,len,0);
				int sockRec=temp.data.fd;
				if(getNum>0)
					thing=CPPSAY;
				else
				{
					*(char*)pget=0;
					thing=CPPOUT;
					epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
					close(temp.data.fd);
				}
				if(pfunc!=NULL)
				{
					if(pfunc(thing,sockRec,getNum,pget,pneed,*this))
						return false;
				}
			}
		}
		return true;
	}
#endif
};
/*******************************
 * author:chenxuan
 * class: client to connect server
 * example:../example/cli-ask/main.cpp
******************************/
class ClientTcpIp{
private:
	int sock;//myself
	sockaddr_in addrC;//server information
	char ip[128];//server Ip
	char* hostip;//host ip
	char* hostname;//host name
	char selfIp[128];
	const char* error;
#ifdef CPPWEB_OPENSSL
	SSL* ssl;
	SSL_CTX* ctx;
#endif
public:
	ClientTcpIp(const char* hostIp,unsigned short port)
	{
		memset(ip,0,128);
		memset(selfIp,0,128);
		error=NULL;
		hostip=(char*)malloc(sizeof(char)*50);
		if(hostip==NULL)
		{
			error="malloc wrong";
			return;
		}
		memset(hostip,0,sizeof(char)*50);
		hostname=(char*)malloc(sizeof(char)*50);
		if(hostname==NULL)
		{
			error="malloc wrong";
			return;
		}		
		memset(hostname,0,sizeof(char)*50);
		if(hostIp!=NULL)
			strcpy(ip,hostIp);
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(hostIp!=NULL)
			addrC.sin_addr.s_addr=inet_addr(hostIp);
		addrC.sin_family=AF_INET;//af_intt IPv4
		addrC.sin_port=htons(port);
#ifdef CPPWEB_OPENSSL
		ssl=NULL;
		ctx=NULL;
#endif
	}
	~ClientTcpIp()
	{
#ifdef CPPWEB_OPENSSL
		if(ssl!=NULL)
		{
			SSL_shutdown(ssl);
			SSL_free(ssl);	
		}
		if(ctx!=NULL)
			SSL_CTX_free(ctx);
#endif
		if(hostip!=NULL)
			free(hostip);
		if(hostname!=NULL)
			free(hostname);
#ifndef _WIN32
		close(sock);
#else
		closesocket(sock);
#endif
	}
	void addHostIp(const char* ip,unsigned short port=0)
	{
		if(ip==NULL)
			return;
		strcpy(this->ip,ip);
		addrC.sin_addr.s_addr=inet_addr(ip);
		if(port!=0)
			addrC.sin_port=htons(port);
	}
	bool tryConnect()
	{
		if(connect(sock,(sockaddr*)&addrC,sizeof(sockaddr))==-1)
			return false;
		return true;
	}
	inline int receiveHost(void* prec,int len,int flag=0)
	{
		return recv(sock,(char*)prec,len,flag);
	}
	int receiveHost(std::string& buffer,int flag=0)
	{
		char temp[1024]={0};
		buffer.clear();
		int len=receiveHost(temp,1024,flag);
		if(len<=0)
			return len;
		while(len==1024)
		{
			buffer.append(temp,1024);
			len=receiveHost(temp,1024,flag);
		}
		return buffer.size();
	}
	inline int sendHost(const void* ps,int len)
	{
		return send(sock,(char*)ps,len,0);
	}
	bool disconnectHost()
	{
#ifndef _WIN32
		close(sock);
#else
		closesocket(sock);
#endif
#ifdef CPPWEB_OPENSSL
		if(ssl!=NULL)
		{
			SSL_shutdown(ssl);
			SSL_free(ssl);	
		}
		ssl=NULL;
#endif
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(sock==-1)
			return false;
		return true;
	}
	char* getSelfIp()
	{
		char name[300]={0};
		gethostname(name,300);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.s_addr,p,phost->h_length);
		memset(selfIp,0,sizeof(char)*100);
		memcpy(selfIp,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return selfIp;
	}
	char* getSelfName(char* hostname,unsigned int bufferLen)
	{
		char name[300]={0};
		gethostname(name,300);
		if(strlen(name)>=bufferLen)
			return NULL;
		memcpy(hostname,name,strlen(name));
		return hostname;
	}
	inline const char* lastError()
	{
		return error;
	}
#ifdef CPPWEB_OPENSSL
	bool tryConnectSSL()
	{
		const SSL_METHOD* meth=SSLv23_client_method();
		if(false==this->tryConnect())
		{
			error="connect wrong";
			return false;
		}
		if(meth==NULL)
		{
			error="ssl init wrong";
			return false;
		}
		ctx=SSL_CTX_new(meth);
		if(ctx==NULL)
		{
			error="ssl new wrong";
			return false;
		}
		ssl=SSL_new(ctx);
		if(NULL==ssl)
		{
			error="ssl new wrong";
			return false;
		}
		SSL_set_fd(ssl,sock);
		int ret=SSL_connect(ssl);
		if(ret==-1)
		{
			error="ssl connect wrong";
			return false;
		}
		return true;
	}
	inline int sendHostSSL(const void* psen,int len)
	{
		return SSL_write(ssl,psen,len);
	}
	inline int receiveHostSSL(void* buffer,int len)
	{
		return SSL_read(ssl,buffer,len);
	}
#endif
	static bool getDnsIp(const char* name,char* ip,unsigned int ipMaxLen)
	{
		hostent* phost=gethostbyname(name);
		if(phost==NULL)
			return false;
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.s_addr,p,phost->h_length);
		if(strlen(inet_ntoa(addr))>=ipMaxLen)
			return false;
		strcpy(ip,inet_ntoa(addr));
		return true;
	}
};
/*******************************
 * author:chenxuan
 * class:deal http request,and create a datagram
 * example:../doc/DealHttp.md
******************************/
class DealHttp{
public:
	friend class HttpServer;
	enum FileKind{
		UNKNOWN=0,HTML=1,TXT=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP=7,JSON=8,
	};
	enum Status{
		STATUSOK=200,STATUSNOCON=204,STATUSMOVED=301,STATUSMOVTEMP=302,
		STATUSBADREQUEST=400,STATUSFORBIDDEN=403,
		STATUSNOFOUND=404,STATUSNOIMPLEMENT=501,
	};
	struct Datagram{
		Status statusCode;
		FileKind typeFile;
		unsigned fileLen;
		std::unordered_map<std::string,std::string> head;
		std::unordered_map<std::string,std::string> cookie;
		std::string typeName;
		std::string body;
		Datagram():statusCode(STATUSOK),typeFile(TXT),fileLen(0){};
	};
	struct Request{
		std::string method;
		std::string askPath;
		std::string version;
		std::unordered_map<std::string,std::string> head;
		const char* body;
		Request():body(NULL){};
	};
private:
	char ask[512];
	char* pfind;
	const char* error;
	const char* connect;
	const char* serverName;
public:
	Datagram gram;//default gram to create gram
	std::unordered_map<std::string,std::string> head;//default head toadd middleware
	std::unordered_map<std::string,std::string> cookie;//default cookie to add middleware
	DealHttp()
	{
		for(int i=0;i<512;i++)
			ask[i]=0;
		pfind=NULL;
		error=NULL;
		connect="keep-alive";
		serverName="LCserver/1.1";
	}
private:
	bool cutLineAsk(char* message,const char* pcutIn)
	{
		if(message==NULL||pcutIn==NULL)
		{
			error="wrong NULL";
			return false;
		}
		char* ptemp=strstr(message,pcutIn);
		if(ptemp==NULL)
			return false;
		while(*(ptemp++)!='\n');
		*ptemp=0;
		return true;
	}
	inline char* findFirst(void* message,const char* ptofind)
	{
		return strstr((char*)message,ptofind);
	}
	const char* getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen)
	{
		char* temp=strstr((char*)message,askWay);
		if(temp==NULL)
			return NULL;
		char format[20]={0};
		sprintf(format,"%%%us",bufferLen);
		sscanf(temp+strlen(askWay)+1,format,buffer);
		return buffer;
	}
	void createTop(FileKind kind,char* ptop,unsigned int bufLen,int* topLen,unsigned int fileLen)
	{
		if(bufLen<100)
		{
			this->error="buffer too short";
			return;
		}
		sprintf(ptop,"HTTP/1.1 200 OK\r\n"
				"Server %s\r\n"
				"Connection: %s\r\n",serverName,connect);
		if(!head.empty())
		{
			for(auto iter=head.begin();iter!=head.end();iter++)
				sprintf(ptop,"%s%s: %s\r\n",\
						ptop,iter->first.c_str(),iter->second.c_str());
			for(auto iter=cookie.begin();iter!=cookie.end();iter++)
				setCookie(ptop,bufLen,iter->first.c_str(),iter->second.c_str());
		}
		switch (kind)
		{
		   case UNKNOWN:
			sprintf(ptop,"%sContent-Length:%d\r\n\r\n",ptop,fileLen);
			break;
		   case HTML:
			sprintf(ptop,"%sContent-Type:text/html\r\n"
					"Content-Length:%d\r\n\r\n",ptop,fileLen);
			break;
		   case TXT:
			sprintf(ptop,"%sContent-Type:text/plain\r\n"
					"Content-Length:%d\r\n\r\n",ptop,fileLen);
			break;
		   case IMAGE:
			sprintf(ptop,"%sContent-Type:image\r\n"
					"Content-Length:%d\r\n\r\n",ptop,fileLen);
			break;
		   case NOFOUND:
			sprintf(ptop,"HTTP/1.1 404 Not Found\r\n"
					"Server %s/1.1\r\n"
					"Connection: %s\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Length:%d\r\n\r\n"
					"404 no found",serverName,connect,(int)strlen("404 no found"));
			break;
		   case CSS:
			sprintf(ptop,"%sContent-Type:text/css\r\n"
					"Content-Length:%d\r\n\r\n",ptop,fileLen);
			break;
		   case JS:
			sprintf(ptop,"%sContent-Type:text/javascript\r\n"
					"Content-Length:%d\r\n\r\n",ptop,fileLen);
			break;
		   case ZIP:
			sprintf(ptop,"%sContent-Type:application/zip\r\n"
					"Content-Length:%d\r\n\r\n",ptop,fileLen);
			break;
		   case JSON:
			sprintf(ptop,"%sContent-Type:application/json\r\n"
					"Content-Length:%d\r\n\r\n",ptop,fileLen);
			break;
		}
		*topLen=strlen(ptop);
	}
	int findFileMsg(const char* pname,int* plen,char* buffer,unsigned int bufferLen)
	{
		FILE* fp=fopen(pname,"rb+");
		unsigned int flen=0,i=0;
		if(fp==NULL)
			return 1;
		fseek(fp,0,SEEK_END);
		flen=ftell(fp);
		if(flen>=bufferLen)
		{
			this->error="buffer too short";
			fclose(fp);
			return 2;
		}
		fseek(fp,0,SEEK_SET);
		for(i=0;i<flen;i++)
			buffer[i]=fgetc(fp);
		buffer[i]=0;
		*plen=flen;
		fclose(fp);
		return 0;
	}
	int getFileLen(const char* pname)
	{
		if(pname==NULL)
			return 0;
		FILE* fp=fopen(pname,"r+");
		int len=0;
		if(fp==NULL)
			return 0;
		fseek(fp,0,SEEK_END);
		len=ftell(fp);
		fclose(fp);
		return len;
	}
	char* findBackString(char* local,int len,char* word,int maxWordLen)
	{
		int i=0;
		char* ptemp=local+len+1;
		char* pend=NULL;
		while(1)//95 _ 
			if((*ptemp>47&&*ptemp<58)||(*ptemp>96&&*ptemp<123)||(*ptemp>64&&*ptemp<91)||*ptemp==95||*ptemp==37)
				break;
			else
				ptemp++;
		pend=ptemp;
		while(1)//46 . 47 / 45 - 43 + 37 % 63 ?
			if((*pend>90&&*pend<97&&*pend!=95)||(*pend<48&&*pend!=46&&*pend!=47&&*pend!=45&&*pend!=43&&*pend!=37)||*pend>122||*pend==63)
				break;
			else
				pend++;
		for(char* pi=ptemp;pi<pend&&i<maxWordLen;pi++)
			word[i++]=*pi;
		word[i]=0;
		return word;
	}
public:
	int createSendMsg(FileKind kind,char* buffer,unsigned int bufferLen,const char* pfile,int* plong)
	{
		int temp=0;
		int len=0,noUse=0,flag=0;
		if(kind==NOFOUND)
		{
			this->createTop(kind,buffer,bufferLen,&temp,len);
			*plong=len+temp+1;
			return 0;
		}
		len=this->getFileLen(pfile);
		if(len==0)
			return 1;
		this->createTop(kind,buffer,bufferLen,&temp,len);
		flag=this->findFileMsg(pfile,&noUse,buffer+temp,bufferLen-temp);
		if(flag==2)
			return 2;
		*plong=len+temp+1;
		return 0;
	}
	const char* analysisHttpAsk(void* message,const char* pneed="GET")
	{
		if(message==NULL)
		{
			error="wrong NULL";
			return NULL;
		}
		pfind=strstr((char*)message,pneed);
		if(pfind==NULL)
			return NULL;
		return this->findBackString(pfind,strlen(pneed),ask,512);
	}
	void* customizeAddTop(void* buffer,unsigned int bufferLen,int statusNum,unsigned int contentLen,const char* contentType="application/json",const char* connection="keep-alive",const char* staEng=NULL)
	{
		const char* statusEng=NULL;
		if(bufferLen<100)
			return NULL;
		switch(statusNum)
		{
		case 200:
			statusEng="OK";
			break;
		case 204:
			statusEng="No Content";
			break;
		case 301:
			statusEng="Moved Permanently";
			break;
		case 400:
			statusEng="Bad Request";
			break;
		case 403:
			statusEng="Forbidden";
			break;
		case 404:
			statusEng="Not Found";
			break;
		case 501:
			statusEng="Not Implemented";
			break;
		default:
			statusEng=staEng;
			break;
		}
		sprintf((char*)buffer,"HTTP/1.1 %d %s\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: %s\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n",statusNum,statusEng,connection,contentType,contentLen);
		return buffer;
	}
	void* customizeAddHead(void* buffer,unsigned int bufferLen,const char* key,const char* value)
	{
		if(strlen((char*)buffer)+strlen(key)+strlen(value)+4>=bufferLen)
			return NULL;
		strcat((char*)buffer,key);
		strcat((char*)buffer,": ");
		strcat((char*)buffer,value);
		strcat((char*)buffer,"\r\n");
		return buffer;
	}
	int customizeAddBody(void* buffer,unsigned int bufferLen,const char* body,unsigned int bodyLen)
	{
		int topLen=0;
		if(buffer==NULL)
			return -1;
		strcat((char*)buffer,"\r\n");
		unsigned int i=0;
		topLen=strlen((char*)buffer);
		if(body==NULL||bodyLen==0)
			return topLen;
		if(bufferLen<topLen+bodyLen)
			return -1;
		char* temp=(char*)buffer+strlen((char*)buffer);
		for(i=0;i<bodyLen;i++)
			temp[i]=body[i];
		temp[i+1]=0;
		return topLen+bodyLen;
	}
	bool setCookie(void* buffer,unsigned int bufferLen,const char* key,const char* value,int liveTime=-1,const char* path=NULL,const char* domain=NULL)
	{
		char temp[1000]={0};
		if(strlen(key)+strlen(value)>1000)
			return false;
		if(strstr(value,"max-age")!=NULL)
		{
			sprintf(temp,"Set-Cookie: %s=%s;",key,value);
			if(strlen((char*)buffer)+strlen(temp)>=bufferLen)
				return false;
			strcat((char*)buffer,temp);
			strcat((char*)buffer,"\r\n");
			return true;
		}
		sprintf(temp,"Set-Cookie: %s=%s;max-age= %d;",key,value,liveTime);
		if(strlen((char*)buffer)+strlen(temp)>=bufferLen)
			return false;
		strcat((char*)buffer,temp);
		if(path!=NULL)
		{
			strcat((char*)buffer,"Path=");
			strcat((char*)buffer,path);
			strcat((char*)buffer,";");
		}
		if(domain!=NULL)
		{
			strcat((char*)buffer,"Domain=");
			strcat((char*)buffer,domain);
			strcat((char*)buffer,";");
		}
		strcat((char*)buffer,"\r\n");
		return true;
	}
	std::string designCookie(const char* value,int liveTime=-1,const char* path=NULL,const char* domain=NULL)
	{
		std::string result="";
		result+=value;
		result+=";";
		result+="max-age="+std::to_string(liveTime);
		result+=";";
		if(path!=NULL)
		{
			result+="Path=";
			result+=path;
			result+=";";
		}
		if(domain!=NULL)
		{
			result+="Domain=";
			result+=domain;
			result+=";";
		}
		return result;
	}
	const char* getCookie(void* recText,const char* key,char* value,unsigned int valueLen)
	{
		if(recText==NULL||key==NULL||value==NULL||valueLen==0)
			return NULL;
		char* temp=strstr((char*)recText,"\r\n\r\n"),*cookie=NULL;
		if(temp==NULL)
			return NULL;
		*temp=0;
		cookie=strstr((char*)recText,"Cookie");
		if(cookie==NULL)
			return NULL;
		cookie=strstr(cookie,key);
		if(cookie==NULL)
			return NULL;
		this->findBackString(cookie,strlen(key),value,valueLen);
		*temp='\r';
		return value;
	}
	int autoAnalysisGet(const char* message,char* psend,unsigned int bufferLen,const char* pfirstFile,int* plen)
	{
		if(bufferLen<128)
			return 2;
		if(NULL==this->analysisHttpAsk((void*)message))
			return 1;
		int temp=0;
		if(strcmp(ask,"HTTP/1.1")==0||strcmp(ask,"HTTP/1.0")==0)
			temp=this->createSendMsg(HTML,psend,bufferLen,pfirstFile,plen);
		else if(strstr(ask,".html"))
			temp=this->createSendMsg(HTML,psend,bufferLen,ask,plen);
		else if(strstr(ask,".txt"))
			temp=this->createSendMsg(TXT,psend,bufferLen,ask,plen);
		else if(strstr(ask,".zip"))
			temp=this->createSendMsg(ZIP,psend,bufferLen,ask,plen);
		else if(strstr(ask,".png")||strstr(ask,".PNG")||strstr(ask,".jpg")||strstr(ask,".jpeg"))
			temp=this->createSendMsg(IMAGE,psend,bufferLen,ask,plen);
		else if(strstr(ask,".css"))
			temp=this->createSendMsg(CSS,psend,bufferLen,ask,plen);
		else if(strstr(ask,".js"))
			temp=this->createSendMsg(JS,psend,bufferLen,ask,plen);
		else if(strstr(ask,".json"))
			temp=this->createSendMsg(JSON,psend,bufferLen,ask,plen);
		else 
			temp=this->createSendMsg(UNKNOWN,psend,bufferLen,ask,plen);
		if(temp!=0)
			this->createSendMsg(NOFOUND,psend,bufferLen,ask,plen);
		return temp;
	}
	const char* getKeyValue(const void* message,const char* key,char* value,unsigned int maxValueLen,bool onlyFromBody=false)
	{
		char* temp=NULL;
		if(onlyFromBody==false)
			temp=strstr((char*)message,key);
		else 
		{
			temp=strstr((char*)message,"\r\n\r\n");
			if(temp==NULL)
				return NULL;
			temp=strstr(temp,key);
		}
		if(temp==NULL)
			return NULL;
		return this->findBackString(temp,strlen(key),value,maxValueLen);
	}
	const char* getKeyLine(const void* message,const char* key,char* line,unsigned int maxLineLen,bool onlyFromBody=false)
	{
		unsigned int i=0;
		char* ptemp=NULL;
		if(false==onlyFromBody)
			ptemp=strstr((char*)message,key);
		else
		{
			ptemp=strstr((char*)message,"\r\n\r\n");
			if(ptemp==NULL)
				return NULL;
			ptemp=strstr(ptemp,key);
		}
		if(ptemp==NULL)
			return NULL;
		ptemp+=strlen(key);
		while(*ptemp==' ')
			ptemp++;
		while(*(ptemp++)!='\r'&&i<maxLineLen)
			line[i++]=*ptemp;
		line[i-1]=0;
		return line;
	}
	const char* getRouteKeyValue(const void* routeMsg,const char* key,char* value,unsigned int valueLen)
	{
		char* temp=strstr((char*)routeMsg,key);
		if(temp==NULL)
			return NULL;
		return this->findBackString(temp,strlen(key),value,valueLen);
	}
	const char* getWildUrl(const void* getText,const char* route,char* buffer,unsigned int maxLen)
	{
		char* temp=strstr((char*)getText,route);
		if(temp==NULL||maxLen==0)
			return NULL;
		temp+=strlen(route);
		if(*temp==' ')
		{
			buffer[0]=0;
			return buffer;
		}
		char format[20]={0};
		sprintf(format,"%%%us",maxLen);
		sscanf(temp,format,buffer);
		return buffer;
	}
	int getRecFile(const void* message,unsigned messageSize,char* fileName,int nameLen,char* buffer,unsigned int bufferLen)
	{
		char tempLen[20]={0},*end=NULL,*top=NULL;
		int result=0;
		if(NULL==this->getKeyLine(message,"boundary",buffer,bufferLen))
			return 0;
		if(NULL==this->getKeyValue(message,"filename",fileName,nameLen))
			return 0;
		if(NULL==this->getKeyValue(message,"Content-Length",tempLen,20))
			return 0;
		if(0>=sscanf(tempLen,"%d",&result))
			return 0;
		if((top=strstr((char*)message,buffer))==NULL)
			return 0;
		if((top=strstr(top+strlen(buffer),buffer))==NULL)
			return 0;
		if((end=(char*)memmem(top+strlen(buffer)\
							  ,messageSize-(top+strlen(buffer)-(char*)message)\
							  ,buffer,strlen(buffer)))==NULL)
			return 0;
		if((top=strstr(top,"\r\n\r\n"))==NULL)
			return 0;
		if(end-top>bufferLen)
			return 0;
		top+=4;
		end-=2;
		result=end-top;
		unsigned int i=0;
		for(i=0;top!=end;i++,top++)
			buffer[i]=*top;
		buffer[i+1]=0;
		return result;
	}
	bool analysisRequest(Request& req,const void* recvText)
	{
		char one[100]={0},two[512]={0},three[512]={0};
		const char* now=(char*)recvText,*end=strstr(now,"\r\n\r\n");
		if(strstr(now,"\r\n\r\n")==NULL)
		{
			this->error="error request";
			return false;
		}
		sscanf(now,"%100s %100s %100s",one,two,three);
		now+=strlen(one)+strlen(two)+strlen(three)+4;
		req.method=one;
		req.askPath=two;
		req.version=three;
		req.body=end+4;
		while(end>now)
		{
			sscanf(now,"%512[^:]: %512[^\r]",two,three);
			if(strlen(two)==512||strlen(three)==512)
			{
				error="head too long";
				return false;
			}
			now+=strlen(two)+strlen(three)+4;
			req.head.insert(std::pair<std::string,std::string>{two,three});
		}
		return true;
	}
	bool createAskRequest(Request& req,void* buffer,unsigned buffLen)
	{
		if(buffLen<200)
		{
			error="buffer len too short";
			return false;
		}
		if(req.head.find("Host")==req.head.end())
		{
			error="cannot not find host";
			return false;
		}
		if(req.method.size()==0)
			req.method="GET";
		if(req.askPath.size()==0)
			req.askPath="/";
		if(req.version.size()==0)
			req.version="HTTP/1.1";
		sprintf((char*)buffer,"%s %s %s\r\n",req.method.c_str(),req.askPath.c_str(),req.version.c_str());
		for(auto iter=req.head.begin();iter!=req.head.end();iter++)
		{
			if(strlen((char*)buffer)+iter->first.size()+iter->second.size()+2>buffLen)
			{
				error="buffer len too short";
				return false;
			}
			strcat((char*)buffer,iter->first.c_str());
			strcat((char*)buffer,": ");
			strcat((char*)buffer,iter->second.c_str());
			strcat((char*)buffer,"\r\n");
		}
		if(req.head.find("Connection")==req.head.end())
			strcat((char*)buffer,"Connection: Close\r\n");
		strcat((char*)buffer,"\r\n");
		if(req.body!=NULL)
			strcat((char*)buffer,req.body);
		return true;
	}
	int createDatagram(const Datagram& gram,void* buffer,unsigned bufferLen)
	{
		if(gram.fileLen>bufferLen||bufferLen==0)
			return -1;
		const char* statusEng=NULL;
		char temp[200]={0};
		if(bufferLen<100||bufferLen<gram.fileLen+100)
		{
			error="len too short";
			return -1;
		}
		switch(gram.statusCode)
		{
		case STATUSOK:
			statusEng="OK";
			break;
		case STATUSNOCON:
			statusEng="No Content";
			break;
		case STATUSMOVED:
			statusEng="Moved Permanently";
			break;
		case STATUSMOVTEMP:
			statusEng="Found";
			break;
		case STATUSBADREQUEST:
			statusEng="Bad Request";
			break;
		case STATUSFORBIDDEN:
			statusEng="Forbidden";
			break;
		case STATUSNOFOUND:
			statusEng="Not Found";
			break;
		case STATUSNOIMPLEMENT:
			statusEng="Not Implemented";
			break;
		default:
			error="status code UNKNOWN";
			return -1;
		}
		sprintf((char*)buffer,"HTTP/1.1 %d %s\r\n"
			"Server %s\r\n"
			"Connection: %s\r\n",
			gram.statusCode,statusEng,serverName,connect);
		if(gram.statusCode==STATUSNOFOUND)
		{
			sprintf((char*)buffer,"%sContent-Type: text/plain\r\n"
					"Content-Length:%zu\r\n\r\n404 page no found"
					,(char*)buffer,strlen("404 page no found"));
			return strlen((char*)buffer);
		}
		switch(gram.typeFile)
		{
		case UNKNOWN:
			if(gram.typeName.size()==0||gram.typeName.size()>200)
			{
				error="type name wrong";
				strcat((char*)buffer,"\r\n");
				return strlen((char*)buffer);
			}
			sprintf(temp,"Content-Type:%s\r\n",gram.typeName.c_str());
			break;
		case NOFOUND:
			strcat((char*)buffer,"\r\n");
			return strlen((char*)buffer);
		case HTML:
			sprintf(temp,"Content-Type:%s\r\n","text/html");
			break;
		case TXT:
			sprintf(temp,"Content-Type:%s\r\n","text/plain");
			break;
		case IMAGE:
			sprintf(temp,"Content-Type:%s\r\n","image");
			break;
		case CSS:
			sprintf(temp,"Content-Type:%s\r\n","text/css");
			break;
		case JS:
			sprintf(temp,"Content-Type:%s\r\n","text/javascript");
			break;
		case JSON:
			sprintf(temp,"Content-Type:%s\r\n","application/json");
			break;
		case ZIP:
			sprintf(temp,"Content-Type:%s\r\n","application/zip");
			break;
		}
		strcat((char*)buffer,temp);
		sprintf(temp,"Content-Length:%d\r\n",gram.fileLen);
		strcat((char*)buffer,temp);
		if(!gram.head.empty())
			for(auto iter=gram.head.begin();iter!=gram.head.end();iter++)
				customizeAddHead(buffer,bufferLen,iter->first.c_str(),iter->second.c_str());
		if(!head.empty())
			for(auto iter=head.begin();iter!=head.end();iter++)
				customizeAddHead(buffer,bufferLen,iter->first.c_str(),iter->second.c_str());
		if(!gram.cookie.empty())
			for(auto iter=gram.cookie.begin();iter!=gram.cookie.end();iter++)
				setCookie(buffer,bufferLen,iter->first.c_str(),iter->second.c_str());
		if(!cookie.empty())
			for(auto iter=cookie.begin();iter!=cookie.end();iter++)
				setCookie(buffer,bufferLen,iter->first.c_str(),iter->second.c_str());
		return customizeAddBody(buffer,bufferLen,gram.body.c_str(),gram.fileLen);
	}
	void changeSetting(const char* connectStatus,const char* serverName)
	{
		if(serverName==NULL||connectStatus==NULL)
			return;
		this->serverName=serverName;
		this->connect=connectStatus;
	}
	inline const char* lastError()
	{
		return error;
	}
	static void dealUrl(const char* url,char* urlTop,char* urlEnd,unsigned int topLen,unsigned int endLen)
	{
		const char* ptemp=NULL;
		char format[20]={0};
		int len=0;
		if((ptemp=strstr(url,"http://"))==NULL)
		{
			if(strstr(url,"https://")!=NULL)
			{
				sprintf(format,"%%%u[^/]",topLen);
				sscanf(url+8,format,urlTop);
				len=strlen(urlTop);
				sprintf(format,"%%%us",endLen);
				sscanf(url+len+8,format,urlEnd);
				return;
			}
			else
			{
				sprintf(format,"%%%u[^/]",topLen);
				sscanf(url,format,urlTop);
				len=strlen(urlTop);
				sprintf(format,"%%%us",endLen);
				sscanf(url+len,format,urlEnd);
				return;
			}
		}
		else
		{
			sprintf(format,"%%%u[^/]",topLen);
			sscanf(url+7,format,urlTop);
			len=strlen(urlTop);
			sprintf(format,"%%%us",endLen);
			sscanf(url+len+7,format,urlEnd);
		}
	}
	static const char* urlDecode(char* srcString)
	{
		char ch=0;
		int temp=0;
		unsigned int srcLen=strlen(srcString);
		char* buffer=(char*)malloc(sizeof(char)*strlen(srcString));
		if(buffer==NULL)
			return NULL;
		memset(buffer,0,sizeof(char)*strlen(srcString));
		for (unsigned int i=0; i<strlen(srcString); i++) 
		{
			if (int(srcString[i])==37) 
			{
				sscanf(srcString+i+1, "%x", &temp);
				ch=(char)temp;
				buffer[strlen(buffer)]=ch;
				buffer[strlen(buffer)+1]=0;
				i=i+2;
			} 
			else 
				buffer[strlen(buffer)]=srcString[i];
		}
		if(srcLen<strlen(buffer))
		{
			free(buffer);
			return NULL;
		}
		strcpy(srcString,buffer);
		free(buffer);
		return srcString;
	}
#ifdef _WIN32
	void *memmem(const void *haystack, size_t haystack_len, 
	    const void * const needle, const size_t needle_len)
	{
		if (haystack == NULL) return NULL; // or assert(haystack != NULL);
		if (haystack_len == 0) return NULL;
		if (needle == NULL) return NULL; // or assert(needle != NULL);
		if (needle_len == 0) return NULL;

		for (const char *h = (const char*)haystack;
			 haystack_len >= needle_len;
			 ++h, --haystack_len) {
			if (!memcmp(h, needle, needle_len)) {
				return (void*)h;
			}
		}
		return NULL;
	}
#endif
};
/*******************************
 * author:chenxuan
 * class:use to send eamil easy
 * example:{
 *  Email email("qq.com");
 *  email.CreateSend
 *  email.emailSend
 * }
******************************/
class Email{
private:
	sockaddr_in their_addr;
	bool isDebug;
	char error[30];
	struct Base64Date6
	{
		unsigned int d4 : 6;
		unsigned int d3 : 6;
		unsigned int d2 : 6;
		unsigned int d1 : 6;
	};
public:
	Email(const char* domain,bool debug=false)
	{
		isDebug=debug;
		memset(error,0,sizeof(char)*30);
		memset(&their_addr, 0, sizeof(their_addr));
		their_addr.sin_family = AF_INET;
		their_addr.sin_port = htons(25);	
		hostent* hptr = gethostbyname(domain);		  
		memcpy(&their_addr.sin_addr.s_addr, hptr->h_addr_list[0], hptr->h_length);
	}
	bool emailSend(const char* sendEmail,const char* passwd,const char* recEmail,const char* body)
	{
	  	int sockfd = 0;
	  	char recBuffer[1000]={0},senBuffer[1000]={0},login[128],pass[128];
		sockfd = socket(PF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			sprintf(error,"open socket wrong");
			return false;
		}
		if (connect(sockfd,(struct sockaddr *)&their_addr, sizeof(struct sockaddr)) < 0)
		{
			sprintf(error,"connect wrong");
			return false;
		}
		memset(recBuffer,0,sizeof(char)*1000);
		while (recv(sockfd, recBuffer, 1000, 0) <= 0)
		{
			if(isDebug)
				printf("reconnecting\n");
#ifndef _WIN32
			sleep(2);
#endif
			sockfd = socket(PF_INET, SOCK_STREAM, 0);
			if (sockfd < 0)
			{
				sprintf(error,"open socket wrong");
				return false;
			}
			if (connect(sockfd,(struct sockaddr *)&their_addr, sizeof(struct sockaddr)) < 0)
			{
				sprintf(error,"connect wrong");
				return false;
			}
			memset(recBuffer, 0, 1000);
		}
		if(isDebug)
			printf("get:%s",recBuffer);
		
		memset(senBuffer, 0, 1000);
		sprintf(senBuffer, "EHLO HYL-PC\r\n");
		send(sockfd, senBuffer, strlen(senBuffer), 0);
		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("EHLO REceive:%s\n",recBuffer);
		
		memset(senBuffer, 0, 1000);
		sprintf(senBuffer, "AUTH LOGIN\r\n");
		send(sockfd, senBuffer, strlen(senBuffer), 0);
		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("Auth Login Receive:%s\n",recBuffer);
		
		memset(senBuffer, 0, 1000);
		sprintf(senBuffer, "%s",sendEmail);
		memset(login, 0, 128);
		EncodeBase64(login, senBuffer, strlen(senBuffer));
		sprintf(senBuffer, "%s\r\n", login);
		send(sockfd, senBuffer, strlen(senBuffer), 0);
		if(isDebug)
			printf("Base64 UserName:%s\n",senBuffer);
		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("User Login Receive:%s\n",recBuffer);

		sprintf(senBuffer, "%s",passwd);//password
		memset(pass, 0, 128);
		EncodeBase64(pass, senBuffer, strlen(senBuffer));
		sprintf(senBuffer, "%s\r\n", pass);
		send(sockfd, senBuffer, strlen(senBuffer), 0);
		if(isDebug)
			printf("Base64 Password:%s\n",senBuffer);

		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("Send Password Receive:%s\n",recBuffer);

		// self email
		memset(recBuffer, 0, 1000);
		sprintf(senBuffer, "MAIL FROM: <%s>\r\n",sendEmail);  
		send(sockfd, senBuffer, strlen(senBuffer), 0);
		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("set Mail From Receive:%s\n",recBuffer);

		// recv email
		sprintf(recBuffer, "RCPT TO:<%s>\r\n", recEmail);
		send(sockfd, recBuffer, strlen(recBuffer), 0);
		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("Tell Sendto Receive:%s\n",recBuffer);
		int bug=0;
		sscanf(recBuffer,"%d",&bug);
		if(bug==550)
		{
			sprintf(error,"recemail wrong");
			return false;
		}
		// send body
		sprintf(senBuffer, "DATA\r\n");
		send(sockfd, senBuffer, strlen(senBuffer), 0);
		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("Send Mail Prepare Receive:%s\n",recBuffer);

		// send data
		sprintf(senBuffer, "%s\r\n.\r\n", body);
		send(sockfd, senBuffer, strlen(senBuffer), 0);
		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("Send Mail Receive:%s\n",recBuffer);

		// QUIT
		sprintf(senBuffer,"QUIT\r\n");
		send(sockfd, senBuffer, strlen(senBuffer), 0);
		memset(recBuffer, 0, 1000);
		recv(sockfd, recBuffer, 1000, 0);
		if(isDebug)
			printf("Quit Receive:%s\n",recBuffer);
		return true;
	}
	char ConvertToBase64(char uc)
	{
		if (uc < 26)
			return 'A' + uc;
		if (uc < 52)
			return 'a' + (uc - 26);
		if (uc < 62)
			return '0' + (uc - 52);
		if (uc == 62)
			return '+';
		return '/';
	}
	void EncodeBase64(char *dbuf, char *buf128, int len)
	{
		struct Base64Date6 *ddd = NULL;
		int i = 0;
		char buf[256] = { 0 };
		char* tmp = NULL;
		char cc = '\0';
		memset(buf, 0, 256);
		strcpy(buf, buf128);
		for (i = 1; i <= len / 3; i++)
		{
			tmp = buf + (i - 1) * 3;
			cc = tmp[2];
			tmp[2] = tmp[0];
			tmp[0] = cc;
			ddd = (struct Base64Date6 *)tmp;
			dbuf[(i - 1) * 4 + 0] = ConvertToBase64((unsigned int)ddd->d1);
			dbuf[(i - 1) * 4 + 1] = ConvertToBase64((unsigned int)ddd->d2);
			dbuf[(i - 1) * 4 + 2] = ConvertToBase64((unsigned int)ddd->d3);
			dbuf[(i - 1) * 4 + 3] = ConvertToBase64((unsigned int)ddd->d4);
		}
		if (len % 3 == 1)
		{
			tmp = buf + (i - 1) * 3;
			cc = tmp[2];
			tmp[2] = tmp[0];
			tmp[0] = cc;
			ddd = (struct Base64Date6 *)tmp;
			dbuf[(i - 1) * 4 + 0] = ConvertToBase64((unsigned int)ddd->d1);
			dbuf[(i - 1) * 4 + 1] = ConvertToBase64((unsigned int)ddd->d2);
			dbuf[(i - 1) * 4 + 2] = '=';
			dbuf[(i - 1) * 4 + 3] = '=';
		}
		if (len % 3 == 2)
		{
			tmp = buf + (i - 1) * 3;
			cc = tmp[2];
			tmp[2] = tmp[0];
			tmp[0] = cc;
			ddd = (struct Base64Date6 *)tmp;
			dbuf[(i - 1) * 4 + 0] = ConvertToBase64((unsigned int)ddd->d1);
			dbuf[(i - 1) * 4 + 1] = ConvertToBase64((unsigned int)ddd->d2);
			dbuf[(i - 1) * 4 + 2] = ConvertToBase64((unsigned int)ddd->d3);
			dbuf[(i - 1) * 4 + 3] = '=';
		}
		return;
	}
	void CreateSend(const char* youName,const char* toName,const char* from,const char* to,const char* subject,const char* body,char* buf)
	{
		sprintf(buf,"From: \"%s\"<%s>\r\n"
				"To: \"%s\"<%s>\r\n"
				"Subject:%s\r\n\r\n"
				"%s\n",youName,from,toName,to,subject,body);
	}
	inline const char* lastError()
	{
		return error;
	}
	static const char* getDomainBySelfEmail(const char* email,char* buffer,int bufferLen)
	{
		char* temp=strrchr((char*)email,'@');
		if(temp==NULL)
			return NULL;
		if(bufferLen<=20)
			return NULL;
		if(strlen(temp)>15)
			return NULL;
		sprintf(buffer,"smtp.%s",temp+1);
		return buffer;
	}
};
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-18 10:18:50
* description:as its name
* example: {
* ThreadPool pool(threadNum);
* ThreadPool::Task task;
* task.arg=(the thing you want to give thread)
* task.ptask=(the function worker)
* pool.addTask(task);
* }
***********************************************/
class ThreadPool{
public://a struct for you to add task
	struct Task{
		void* (*ptask)(void*);
		void* arg;
	};
private:
	std::queue<Task> thingWork;//a queue for struct task
	pthread_cond_t condition;//a condition mutex
	pthread_mutex_t lockPool;//a lock to lock queue
	pthread_mutex_t lockTask;//a lock for user to ctrl
	pthread_mutex_t lockBusy;//a lock for busy thread
	pthread_t* thread;//an array for thread
	unsigned int liveThread;//num for live thread
	unsigned int busyThread;//num for busy thread
	bool isContinue;//if the pool is continue
private:
	static void* worker(void* arg)//the worker for user 
	{
		ThreadPool* pool=(ThreadPool*)arg;
		while(1)
		{
			pthread_mutex_lock(&pool->lockPool);
			while(pool->isContinue==true&&pool->thingWork.size()==0)
				pthread_cond_wait(&pool->condition,&pool->lockPool);
			if(pool->isContinue==false)
			{
				pthread_mutex_unlock(&pool->lockPool);
				pthread_exit(NULL);
			}
			if(pool->thingWork.size()>0)
			{
				pthread_mutex_lock(&pool->lockBusy);
				pool->busyThread++;
				pthread_mutex_unlock(&pool->lockBusy);
				ThreadPool::Task task=pool->thingWork.front();
				pool->thingWork.pop();
				pthread_mutex_unlock(&pool->lockPool);
				task.ptask(task.arg);
				pthread_mutex_lock(&pool->lockBusy);
				pool->busyThread--;
				pthread_mutex_unlock(&pool->lockBusy);
			}
		}
		return NULL;
	}
public:
	ThreadPool(unsigned int threadNum=10)//create threads
	{
		if(threadNum<1)
			threadNum=10;
		thread=new pthread_t[threadNum];
		if(thread==NULL)
			return;
		for(unsigned int i=0;i<threadNum;i++)
			thread[i]=0;
		pthread_cond_init(&condition,NULL);
		pthread_mutex_init(&lockPool,NULL);
		pthread_mutex_init(&lockTask,NULL);
		pthread_mutex_init(&lockBusy,NULL);
		liveThread=threadNum;
		isContinue=true;
		busyThread=0;
		for(unsigned int i=0;i<threadNum;i++)
			pthread_create(&thread[i],NULL,worker,this);
	}
	~ThreadPool()//destory pool
	{
		if(isContinue==false)
			return;
		stopPool();
	}
	void threadExit()// a no use funtion
	{
		pthread_t pid=pthread_self();
		for(unsigned int i=0;i<liveThread;i++)
			if(pid==thread[i])
			{
				thread[i]=0;
				break;
			}
		pthread_exit(NULL);
	}
	void addTask(Task task)//by this you can add task
	{
		if(isContinue==false)
			return;
		pthread_mutex_lock(&this->lockPool);
		this->thingWork.push(task);
		pthread_mutex_unlock(&this->lockPool);
		pthread_cond_signal(&this->condition);
	}
	void stopPool()//user delete the pool
	{
		isContinue=false;
		for(unsigned int i=0;i<liveThread;i++)
			pthread_cond_signal(&condition);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_join(thread[i],NULL);
		pthread_cond_destroy(&condition);
		pthread_mutex_destroy(&lockPool);
		pthread_mutex_destroy(&lockTask);
		pthread_mutex_destroy(&lockBusy);
		delete[] thread;
	}
	void getBusyAndTask(unsigned int* pthread,unsigned int* ptask)//get busy live and task num
	{
		pthread_mutex_lock(&lockBusy);
		*pthread=busyThread;
		pthread_mutex_unlock(&lockBusy);
		pthread_mutex_lock(&lockPool);
		*ptask=thingWork.size();
		pthread_mutex_unlock(&lockPool);
	}
	inline void mutexLock()//user to lock ctrl
	{
		pthread_mutex_lock(&this->lockTask);
	}
	inline void mutexUnlock()//user to lock ctrl
	{
		pthread_mutex_unlock(&this->lockTask);
	}
	static pthread_t createPthread(void* arg,void* (*pfunc)(void*))//create a thread 
	{
		pthread_t thread=0;
		pthread_create(&thread,NULL,pfunc,arg);
		return thread;
	}
	static inline void waitPthread(pthread_t thread,void** preturn=NULL)//wait the thread end
	{
		pthread_join(thread,preturn);
	}
	static void createDetachPthread(void* arg,void* (*pfunc)(void*))//create a ddetach thread
	{
		pthread_t thread=0;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_create(&thread,&attr,pfunc,arg);
	}
};
/*******************************
 * author:chenxuan
 * class:class for log is use threadpool and it can write 3000000 every sec;
 * example:{
 * LogSystem log("save name");
 * log.accessLog("str of write");
 * }
******************************/
class LogSystem{
private:
	const char* fileName;
	char* buffer[4];
	const char* error;
	char* now;
	char* page;
	size_t nowLen;
	size_t bufferLen;
	ThreadPool pool;
	std::queue<char*> nowFree;
public:
	LogSystem(const char* name,size_t bufferLen=1024)\
										   :fileName(name),now(NULL),pool(1)
	{
		page=NULL;
		if(fileName==NULL)
			fileName="access.log";
		if(bufferLen<128)
			bufferLen=128;
		this->bufferLen=bufferLen;
		for(unsigned i=0;i<4;i++)
			buffer[i]=NULL;
		for(unsigned i=0;i<4;i++)
		{
			buffer[i]=(char*)malloc(sizeof(char)*bufferLen);
			if(buffer[i]==NULL)
			{
				error="malloc wrong";
				return;
			}
			nowFree.push(buffer[i]);
			memset(buffer[i],0,sizeof(char)*bufferLen);
		}
		now=nowFree.front();
		nowFree.pop();
		page=now;
	}
	~LogSystem()
	{
		std::pair<LogSystem*,char*>* argv=new std::pair<LogSystem*,char*>(this,page);
		worker(argv);
		for(unsigned i=0;i<4;i++)
			if(buffer[i]!=NULL)
				free(buffer[i]);
	}
	void forceWrite()
	{
		while(nowFree.empty());
		pool.mutexLock();
		now=nowFree.front();
		nowFree.pop();
		pool.mutexUnlock();
		nowLen=0;
		std::pair<LogSystem*,char*>* argv=new std::pair<LogSystem*,char*>(this,page);
		ThreadPool::Task task{worker,argv};
		pool.addTask(task);
		page=now;
	}
	void accessLog(const char* text)
	{
		if(text==NULL)
			return;
		if(nowLen+strlen(text)>=bufferLen)
		{
			while(nowFree.empty());
			pool.mutexLock();
			now=nowFree.front();
			nowFree.pop();
			pool.mutexUnlock();
			nowLen=0;
			std::pair<LogSystem*,char*>* argv=new std::pair<LogSystem*,char*>(this,page);
			ThreadPool::Task task{worker,argv};
			pool.addTask(task);
			page=now;
		}
		strcat(now,text);
		strcat(now,"\n");
		nowLen+=strlen(text)+1;
	}
	void operator()(const void* text,int soc)
	{
		recordMessage(text,soc);
	}
	void recordMessage(const void* text,int soc)
	{
		static char method[64]={0},askPath[256]={0},buffer[512]={0},nowTime[48]={0};
		int port=0;
		time_t now=time(NULL);
		strftime(nowTime,48,"%Y-%m-%d %H:%M",localtime(&now));
		if(soc>0)
		{
			memset(askPath,0,sizeof(char)*256);
			sscanf((char*)text,"%64s%255s",method,askPath);
			if(strlen(askPath)==0)
				strcpy(askPath,"no found");
			sprintf(buffer,"%s %s %s %s",nowTime,ServerTcpIp::getPeerIp(soc,&port),method,askPath);
		}
		else if(soc==-1)
		{
			std::pair<LogSystem*,char*>* argv=new std::pair<LogSystem*,char*>(this,page);
			worker(argv);
		}
		else
			sprintf(buffer,"%s localhost %s",nowTime,(char*)text);
		this->accessLog(buffer);
	}
	static void recordRequest(const void* text,int soc)
	{
		static LogSystem loger("access.log");
		loger.recordMessage(text,soc);
	}
	static bool recordFileError(const char* fileName)
	{
		FILE* fp=fopen("wrong.log","r+");
		if(fp==NULL)
			fp=fopen("wrong.log","w+");
		if(fp==NULL)
			return false;
		fseek(fp,0,SEEK_END);
		fprintf(fp,"open file %s wrong\n",fileName);
		fclose(fp);
		return true;
	}
	static void httpLog(const void * text,int soc)
	{
		DealHttp http;
		int port=0;
		FILE* fp=fopen("ask.log","a+");
		if(fp==NULL)
			fp=fopen("ask.log","w+");
		if(fp==NULL)
			return ;
		DealHttp::Request req;
		http.analysisRequest(req,text);
		time_t now=time(NULL);
		fprintf(fp,"%s %s %s %s\n",ctime(&now),ServerTcpIp::getPeerIp(soc,&port),req.method.c_str(),req.askPath.c_str());
		fclose(fp);
		return ;
	}
private:
	static void* worker(void* argv)
	{
		std::pair<LogSystem*,char*>& now=*(std::pair<LogSystem*,char*>*)argv;
		FILE* fp=fopen(now.first->fileName,"a+");
		if(fp==NULL)
		{
			now.first->error="open file wrong";
			return NULL;
		}
		fprintf(fp,"%s",now.second);
		fclose(fp);
		memset(now.second,0,now.first->bufferLen);
		now.first->pool.mutexLock();
		now.first->nowFree.push(now.second);
		now.first->pool.mutexUnlock();
		delete (std::pair<LogSystem*,char*>*)argv;
		return NULL;
	}
};
/*******************************
 * author:chenxuan
 * class:the main class for create server
 * example:../doc/HttpServer.md
******************************/
class HttpServer:private ServerTcpIp{
private:
	enum RouteType{//oneway stand for like /hahah,wild if /hahah/*,static is recource static
		ONEWAY,WILD,STATIC,STAWILD
	};
public://main class for http server2.0
	enum RunModel{//the server model of run
		FORK,MULTIPLEXING,THREAD
	};
#ifndef _WIN32
	enum AskType{//different ask ways in http
		GET,POST,PUT,DELETE,OPTIONS,CONNECT,ALL,
	};
#else
	enum AskType{//different ask ways in http
		GET,POST,PUT,WINDELETE,OPTIONS,CONNECT,ALL,
	};
#endif
	struct RouteFuntion{//inside struct,pack for handle
		AskType ask;
		RouteType type;
		char route[128];
		char pathExtra[128];
		void (*pfunc)(HttpServer&,DealHttp&,int);
	};
private:
	RouteFuntion* arrRoute;
	RouteFuntion* pnowRoute;
	void* getText;
	void* senText;
	const char* defaultFile;
	unsigned int senLen;
	unsigned int recLen;
	unsigned int maxLen;
	unsigned int maxNum;
	unsigned int now;
	unsigned int boundPort;
	unsigned int selfLen;
	int textLen;
	bool isDebug;
	bool isLongCon;
	bool selfCtrl;
	bool isContinue;
	bool isAutoAnalysis;
	void (*middleware)(HttpServer&,DealHttp&,int num);
	void (*clientIn)(HttpServer&,int num,void* ip,int port);
	void (*clientOut)(HttpServer&,int num,void* ip,int port);
	void (*logFunc)(const void*,int);
	void (*logError)(const void*,int);
	Trie<RouteFuntion> trie;
	ThreadPool* pool;
	RunModel model;
public:
	HttpServer(unsigned port,bool debug=false,RunModel serverModel=MULTIPLEXING,unsigned threadNum=5)
		:ServerTcpIp(port),model(serverModel)
	{
		pool=NULL;
		getText=NULL;
		senText=NULL;
		middleware=NULL;
		defaultFile=NULL;
		arrRoute=NULL;
		clientIn=NULL;
		clientOut=NULL;
		logFunc=NULL;
		logError=NULL;
		pnowRoute=NULL;
		senLen=1024*1024;
		maxLen=10;
		recLen=2048;
		selfLen=0;
		boundPort=port;
		isDebug=debug;
		selfCtrl=false;
		isLongCon=true;
		isContinue=true;
		isAutoAnalysis=true;
		textLen=0;
		now=0;
		maxNum=20;
		arrRoute=(RouteFuntion*)malloc(sizeof(RouteFuntion)*20);
		if(arrRoute==NULL)
		{
			this->error=" server route wrong";
			if(logError!=NULL)
				logError(this->error,0);
		}
		else
			memset(arrRoute,0,sizeof(RouteFuntion)*20);
		if(this->model==THREAD)
		{
			if(threadNum==0)
			{
				error="thread num is zero";
				return;
			}
			pool=new ThreadPool(threadNum);
			if(pool==NULL)
			{
				error="pool new wrong";
				return;
			}
		}
#ifndef _WIN32
		if(this->model==FORK)
			signal(SIGCHLD,sigCliDeal);
#endif
	}
	~HttpServer()
	{
		if(arrRoute!=NULL)
			free(arrRoute);
	}
#ifdef CPPWEB_OPENSSL
	inline bool loadKeyCert(const char* certPath,const char* keyPath,const char* passwd=NULL)
	{
		return loadCertificate(certPath,keyPath,passwd);
	}
#endif
	void changeModel(RunModel model,unsigned threadNum=5)
	{
		if(this->model==model&&model!=THREAD)
			return;
		this->model=model;
		if(this->model==THREAD)
		{
			if(threadNum==0)
			{
				error="thread num is zero";
				return;
			}
			if(pool!=NULL)
				delete pool;
			pool=new ThreadPool(threadNum);
			if(pool==NULL)
			{
				error="pool new wrong";
				return;
			}
		}
#ifndef _WIN32
		if(this->model==FORK)
			signal(SIGCHLD,sigCliDeal);
#endif
	}
	bool routeHandle(AskType ask,const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
	{//add route handle in all ask type 
		if(strlen(route)>100)
			return false;
		RouteFuntion* nowRoute=addRoute();
		if(nowRoute==NULL)
			return false;
		nowRoute->ask=ask;
		strcpy(nowRoute->route,route);
		nowRoute->pfunc=pfunc;
		if(route[strlen(route)-1]=='*')
		{
			nowRoute->route[strlen(route)-1]=0;
			nowRoute->type=WILD;
		}
		else
			nowRoute->type=ONEWAY;
		if(false==trie.insert(nowRoute->route,nowRoute))
		{
			error="server:route wrong char";
			if(logError!=NULL)
				logError(this->error,0);
			return false;
		}
		return true;
	}
	bool redirect(const char* route,const char* location)
	{//301 redirect
		if(strlen(route)>100)
			return false;
		RouteFuntion* nowRoute=addRoute();
		if(nowRoute==NULL)
			return false;
		nowRoute->type=STATIC;
		nowRoute->ask=ALL;
		strcpy(nowRoute->route,route);
		strcpy(nowRoute->pathExtra,location);
		if(nowRoute->route[strlen(nowRoute->route)-1]=='*')
		{
			nowRoute->route[strlen(nowRoute->route)-1]=0;
			nowRoute->type=STAWILD;
		}
		nowRoute->pfunc=rediectGram;
		if(false==trie.insert(nowRoute->route,nowRoute))
		{
			error="server:route wrong char";
			if(logError!=NULL)
				logError(this->error,0);
			return false;
		}
		return true;

	}
	bool loadStatic(const char* route,const char* staticFile)
	{//load file such as / -> index.html
		if(strlen(route)>100)
			return false;
		RouteFuntion* nowRoute=addRoute();
		if(nowRoute==NULL)
			return false;
		nowRoute->type=STATIC;
		nowRoute->ask=GET;
		strcpy(nowRoute->route,route);
		strcpy(nowRoute->pathExtra,staticFile);
		if(nowRoute->route[strlen(nowRoute->route)-1]=='*')
		{
			nowRoute->route[strlen(nowRoute->route)-1]=0;
			nowRoute->type=STAWILD;
		}
		nowRoute->pfunc=loadFile;
		if(false==trie.insert(nowRoute->route,nowRoute))
		{
			error="server:route wrong char";
			if(logError!=NULL)
				logError(this->error,0);
			return false;
		}
		return true;
	}
	bool deletePath(const char* route)
	{// forbidden the file path
		if(strlen(route)>100&&route!=NULL)
			return false;
		RouteFuntion* nowRoute=addRoute();
		if(nowRoute==NULL)
			return false;
		nowRoute->type=STAWILD;
		nowRoute->ask=GET;
		if(route[0]=='.'&&route[1]=='/'&&strlen(route)>2)
			strcpy(nowRoute->route,route+2);
		else if(route[0]!='/')
		{
			strcpy(nowRoute->route,"/");
			strcat(nowRoute->route,route);
		}
		else
			strcpy(nowRoute->route,route);
		nowRoute->pfunc=deleteFile;
		if(false==trie.insert(nowRoute->route,nowRoute))
		{
			error="server:route wrong char";
			if(logError!=NULL)
				logError(this->error,0);
			return false;
		}
		return true;
	}
	inline bool get(const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
	{//add routeHandle and ask type is get
		return routeHandle(GET,route,pfunc);
	}
	inline bool post(const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
	{//the same to last funtion
		return routeHandle(POST,route,pfunc);
	}
	inline bool all(const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
	{//receive all ask type
		return routeHandle(ALL,route,pfunc);
	}
	inline void clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
	{//when client in ,it will be call
		clientIn=pfunc;
	}
	inline void clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
	{//when client out ,itwill be call
		clientOut=pfunc;
	}
	bool setMiddleware(void (*pfunc)(HttpServer&,DealHttp&,int))
	{//middleware funtion after get text it will be called
		if(middleware!=NULL)
			return false;
		middleware=pfunc;
		return true;
	}
	inline void continueNext(int cliSock)
	{//middleware funtion to continue default task
		if(middleware==NULL)
			return;
		auto temp=middleware;
		middleware=NULL;
		func(cliSock);
		middleware=temp;
	}
	void setLog(void (*pfunc)(const void*,int),void (*errorFunc)(const void*,int))
	{//log system 
		logFunc=pfunc;
		logError=errorFunc;
	}
	void run(const char* defaultFile=NULL)
	{//server begin to run
		char* getT=(char*)malloc(sizeof(char)*recLen);
		char* sen=(char*)malloc(sizeof(char)*senLen);
		if(sen==NULL||getT==NULL)
		{
			this->error="server:malloc get and sen wrong";
			if(logError!=NULL)
				logError(error,0);
			return;
		}
		memset(getT,0,sizeof(char)*recLen);
		memset(sen,0,sizeof(char)*senLen);
		if(false==this->bondhost())
		{
			this->error="server:bound wrong";
			if(logError!=NULL)
				logError(error,0);
			return;
		}
		if(false==this->setlisten())
		{
			this->error="server:set listen wrong";
			if(logError!=NULL)
				logError(error,0);
			return;
		}
		if(logFunc!=NULL)
			logFunc("server start",0);
		this->getText=getT;
		this->senText=sen;
		this->defaultFile=defaultFile;
		if(isDebug)
			messagePrint();
		switch(model)
		{
		case FORK:
		case MULTIPLEXING:
			while(isContinue)
				this->epollModel(epollHttp,this);
			break;
		case THREAD:
			while(isContinue)
				this->threadHttp();
			break;
		}
		free(sen);
		free(getT);
	}
	int httpSend(int num,void* buffer,int sendLen,int flag=0)
	{
		return this->sendSocket(num,buffer,sendLen,flag);
	}
	int httpRecv(int num,void* buffer,int bufferLen,int flag=0)
	{
		return this->receiveSocket(num,buffer,bufferLen,flag);
	}
	int httpRecv(int num,std::string& buffer,int flag)
	{
		return this->receiveSocket(num,buffer,flag);
	}
	int getCompleteMessage(int sockCli)
	{//some time text is not complete ,it can get left text 
		void*& message=getText;
		unsigned messageLen=this->getRecLen();
		if(message==NULL||message==0)
			return -1;
		unsigned int len=0,old=messageLen;
		char* temp=NULL;
		if((temp=strstr((char*)message,"Content-Length"))==NULL)
			return -1;
		if(sscanf(temp+strlen("Content-Length")+1,"%d",&len)<=0)
			return -1;
		if((temp=strstr((char*)message,"\r\n\r\n"))==NULL)
			return -1;
		temp+=4;
		if(strlen(temp)>=len)
			return messageLen;
		long int leftLen=len-(messageLen-(temp-(char*)message)),getLen=1,all=0;
		while(len+messageLen>recLen)
		{
			recLen*=2;
			message=enlargeMemory(message,recLen);
		}
		unsigned result=messageLen;
		while(leftLen>5&&getLen>0)
		{
			getLen=this->receiveSocket(sockCli,(char*)message+old+all,recLen-old-all);
			result+=getLen;
			all+=getLen;
			leftLen-=getLen;
		}
		textLen=result;
		return result;
	}
	void changeSetting(bool debug,bool isLongCon,bool isAuto=true,unsigned maxSendLen=10)
	{//change setting
		this->isDebug=debug;
		this->isLongCon=isLongCon;
		this->isAutoAnalysis=isAuto;
		if(maxSendLen>1)
			this->maxLen=maxSendLen;
	}
	inline void* recText()
	{//get the recv text;
		return this->getText;
	}
	inline int getRecLen()
	{//get the recv text len
		return this->textLen;
	}
	inline const char* lastError()
	{//get the error of server
		return error;
	}
	inline void disconnect(int soc)
	{
		this->closeSocket(soc);
	}
	inline void selfCreate(unsigned senLen)
	{//use getSenBuff to create task by self
		selfCtrl=true;
		selfLen=senLen;
	}
	inline void* getSenBuffer()
	{//get the sen buffer
		return senText;
	}
	inline unsigned getMaxSenLen()
	{//get sen buffer size
		return this->senLen;
	}
	inline void stopServer()
	{//stop server run;
		this->isContinue=false;
	}
	inline RouteFuntion* getNowRoute()
	{//get the now route;
		return pnowRoute;
	}
	inline void* enlagerSenBuffer()
	{//Proactively scale up sen buffer
		this->senText=enlargeMemory(this->senText,this->senLen);
		return this->senText;
	}
private:
	void messagePrint()
	{
		printf("welcome to web server,the server is runing\n");
		switch(model)
		{
		case MULTIPLEXING:
			printf("model:\t\tIO Multiplexing\n");
			break;
		case THREAD:
			printf("model:\t\tThread Pool\n");
			break;
		case FORK:
			printf("model:\t\tProcess Pool\n");
			break;
		}
		printf("port:\t\t%u\n",boundPort);
		if(isAutoAnalysis)
			printf("auto:\t\tTrue\n");
		else
			printf("auto:\t\tFalse\n");
#ifdef CPPWEB_OPENSSL
		printf("ssl:\t\tTrue\n");
#else
		printf("ssl:\t\tFalse\n");
#endif
		if(defaultFile!=NULL)
			printf("/\t\t->\t%s\n",defaultFile);
		for(unsigned i=0;i<now;i++)
		{
			switch(arrRoute[i].type)
			{
			case ONEWAY:
				printf("%s\t\t->\t",arrRoute[i].route);
				break;
			case WILD:
				printf("%s*\t\t->\t",arrRoute[i].route);
				break;
			case STATIC:
			case STAWILD:
				if(arrRoute[i].pfunc==loadFile)
				{
					if(arrRoute[i].type==STATIC)
						printf("%s\t\t->\t%s\n",arrRoute[i].route,arrRoute[i].pathExtra);
					else
						printf("%s*\t\t->\t%s\n",arrRoute[i].route,arrRoute[i].pathExtra);
				}
				else if(arrRoute[i].pfunc==deleteFile)
				{
					if(arrRoute[i].type==STATIC)
						printf("%s\t\t->\tdelete\n",arrRoute[i].route);
					else
						printf("%s*\t\t->\tdelete\n",arrRoute[i].route);
				}
				else if(arrRoute[i].pfunc==rediectGram)
				{
					if(arrRoute[i].type==STATIC)
						printf("%s\t\t->\t%s\n",arrRoute[i].route,arrRoute[i].pathExtra);
					else
						printf("%s*\t\t->\t%s\n",arrRoute[i].route,arrRoute[i].pathExtra);
				}
				else
					printf("undefine funtion please check the server\n");
				continue;
			}
			switch(arrRoute[i].ask)
			{
			case GET:
				printf("GET\n");
				break;
			case POST:
				printf("POST\n");
				break;
			case ALL:
				printf("All\n");
				break;
			case PUT:
				printf("PUT\n");
				break;
#ifndef _WIN32
			case DELETE:
#else
			case WINDELETE:
#endif
				printf("DELETE\n");
				break;
			case OPTIONS:
				printf("OPTIONS\n");
				break;
			case CONNECT:
				printf("CONNECT\n");
				break;
			}
		}
		if(middleware!=NULL)
			printf("middleware\t\tfuntion set\n");
		if(logFunc!=NULL)
			printf("log\t\tfunction set\n");
		if(logError!=NULL)
			printf("error\t\tfuntion set\n");
		if(clientIn!=NULL)
			printf("clientIn\t\tfunction set\n");
		if(clientOut!=NULL)
			printf("clientout\t\tfunction set\n");
		printf("\n");
	}
	int func(int num)
	{
		static DealHttp http;
		AskType type=GET;
		int len=0,flag=2;
		char ask[200]={0};
		if(middleware!=NULL)
		{
			middleware(*this,http,num);
			return 0;
		}
		if(isLongCon==false)
			http.changeSetting("Close","LCserver/1.1");
		sscanf((char*)this->getText,"%100s",ask);
		if(strstr(ask,"GET")!=NULL)
		{
			http.getAskRoute(this->getText,"GET",ask,200);
			if(isDebug)
				printf("Get url:%s\n",ask);
			type=GET;
		}
		else if(strstr(ask,"POST")!=NULL)
		{
			http.getAskRoute(this->getText,"POST",ask,200);
			if(isDebug)
				printf("POST url:%s\n",ask);
			type=POST;
		}
		else if(strstr(ask,"PUT")!=NULL)
		{
			http.getAskRoute(this->getText,"PUT",ask,200);
			if(isDebug)
				printf("PUT url:%s\n",ask);
			type=PUT;
		}
		else if(strstr(ask,"DELETE")!=NULL)
		{
			http.getAskRoute(this->getText,"DELETE",ask,200);
			if(isDebug)
				printf("DELETE url:%s\n",ask);
#ifndef _WIN32
			type=DELETE;
#else
			type=WINDELETE;
#endif
		}
		else if(strstr(ask,"OPTIONS")!=NULL)
		{
			http.getAskRoute(this->getText,"OPTIONS",ask,200);
			if(isDebug)
				printf("OPTIONS url:%s\n",ask);
			type=OPTIONS;
		}
		else if(strstr(ask,"CONNECT")!=NULL)
		{
			http.getAskRoute(this->getText,"CONNECT",ask,200);
			if(isDebug)
				printf("CONNECT url:%s\n",ask);
			type=CONNECT;
		}
		else 
		{
			memset(ask,0,sizeof(char)*200);
			if(isDebug)
				printf("way not support\n");
			if(logError!=NULL)
				logError(ask,num);
			type=GET;
		}
		void (*pfunc)(HttpServer&,DealHttp&,int)=NULL;
		RouteFuntion* tempRoute=trie.search(ask,[=](const RouteFuntion* now,bool isLast)->bool{
					if(now->ask==ALL||now->ask==type)
					{
						if(isLast&&(now->type==STATIC||now->type==ONEWAY))
							return true;
						else if(now->type==WILD||now->type==STAWILD)
							return true;
						else
							return false;
					}
					return false;
					});
		if(tempRoute!=NULL)
		{
			pnowRoute=tempRoute;
			pfunc=tempRoute->pfunc;
		}
		if(pfunc!=NULL)
		{
			if(pfunc!=loadFile&&pfunc!=deleteFile)
			{
				http.gram.body="";
				http.gram.cookie.clear();
				http.gram.head.clear();
				http.gram.fileLen=0;
				http.gram.typeFile=DealHttp::TXT;
				pfunc(*this,http,num);
				if(selfCtrl)
				{
					selfCtrl=false;
					len=selfLen;
					selfLen=0;
				}
				else
				{
					if(http.gram.fileLen==0)
						http.gram.fileLen=http.gram.body.size();
					len=http.createDatagram(http.gram,this->senText,this->senLen);
					while(len==-1)
					{
						this->senText=enlargeMemory(this->senText,this->senLen);
						len=http.createDatagram(http.gram,this->senText,this->senLen);
					}
				}
			}
			else
			{
				pfunc(*this,http,senLen);
				len=staticLen(-1);
			}
		}
		else if(isAutoAnalysis)
		{
			if(isDebug)
				printf("http:%s\n",http.analysisHttpAsk(this->getText));
			if(http.analysisHttpAsk(this->getText)!=NULL)
			{
				strcpy(ask,http.analysisHttpAsk(this->getText));
				do{
					flag=http.autoAnalysisGet((char*)this->getText,(char*)this->senText,senLen,defaultFile,&len);
					if(flag==2&&this->senLen<maxLen*1024*1024)
						this->enlagerSenBuffer();
				}while(flag==2&&this->senLen<maxLen*1024*1024);
			}
			if(flag==1)
			{
				if(isDebug)
					printf("404 get %s wrong\n",ask);
				if(logError!=NULL)
					logError(ask,num);
			}
			else if(flag==2)
			{
				if(logError!=NULL)
					logError("memory wrong",0);
				http.createSendMsg(DealHttp::NOFOUND,(char*)this->senText,senLen,NULL,&len);
			}
		}
		else
			http.createSendMsg(DealHttp::NOFOUND,(char*)this->senText,senLen,NULL,&len);
		if(len==0)
			http.createSendMsg(DealHttp::NOFOUND,(char*)this->senText,senLen,NULL,&len);
		if(0>=this->sendSocket(num,this->senText,len))
		{
			if(isDebug)
				perror("send wrong");
		}
		else
		{
			if(isDebug)
				printf("200 ok send success\n\n");
		}
		return 0;
	}
	static int epollHttp(Thing thing,int soc,ServerTcpIp&,void* pserver)
	{
		HttpServer& server=*(HttpServer*)pserver;
		int port=0;
		if(thing==CPPIN)
		{
			if(server.clientIn!=NULL)
			{
				strcpy((char*)server.getText,ServerTcpIp::getPeerIp(soc,&port));
				server.clientIn(server,soc,server.getText,port);
			}
			return 0;
		}
		else
		{
			memset(server.getText,0,sizeof(char)*server.recLen);
			int getNum=server.receiveSocket(soc,(char*)server.getText,server.recLen,MSG_DONTWAIT);
			int all=getNum;
			while((int)server.recLen==all)
			{
				server.getText=server.enlargeMemory(server.getText,server.recLen);
				getNum=server.receiveSocket(soc,(char*)server.getText+all,server.recLen-all,MSG_DONTWAIT);
				if(getNum<=0)
					break;
				all+=getNum;
			}
			if(all>0)
			{
				server.textLen=all;
				if(server.model==MULTIPLEXING)
					server.func(soc);
				else
				{
#ifndef _WIN32
					if(fork()==0)
					{
						server.closeSocket(server.sock);
						server.func(soc);
						server.closeSocket(soc);
						free(server.getText);
						free(server.senText);
						exit(0);
					}
#else
					server.func(soc);
#endif
				}
				if(server.logFunc!=NULL)
					server.logFunc(server.recText(),soc);
				if(server.isLongCon==false)
					return soc;
			}
			else
			{
				if(server.clientOut!=NULL)
				{
					int port=0;
					strcpy((char*)server.getText,server.getPeerIp(soc,&port));
					server.clientOut(server,soc,server.getText,port);
				}
				return soc;
			}

		}
		return 0;
	}
#ifndef _WIN32
	void epollHttp()
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,this->getText is rec,len is the max len of this->getText,pneed is others things
		memset(this->getText,0,sizeof(char)*this->recLen);
		int eventNum=epoll_wait(epfd,pevent,512,-1);
		for(int i=0;i<eventNum;i++)
		{
			epoll_event temp=pevent[i];
			if(temp.data.fd==sock)
			{
				sockaddr_in newaddr={0,0,{0},{0}};
				int newClient=acceptSocket(newaddr);
				nowEvent.data.fd=newClient;
				nowEvent.events=EPOLLIN;
				epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
				if(this->clientIn!=NULL)
				{
					strcpy((char*)this->getText,inet_ntoa(newaddr.sin_addr));
					clientIn(*this,newClient,this->getText,newaddr.sin_port);
				}
			}
			else
			{
				int getNum=receiveSocket(temp.data.fd,(char*)this->getText,this->recLen,MSG_DONTWAIT);
				int all=getNum;
				while((int)this->recLen==all)
				{
					this->getText=enlargeMemory(this->getText,this->recLen);
					getNum=receiveSocket(temp.data.fd,(char*)this->getText+all,this->recLen-all,MSG_DONTWAIT);
					if(getNum<=0)
						break;
					all+=getNum;
				}
				if(all>0)
				{
					this->textLen=all;
					if(model==MULTIPLEXING)
						func(temp.data.fd);
					else
					{
						if(fork()==0)
						{
							closeSocket(sock);
							func(temp.data.fd);
							closeSocket(temp.data.fd);
							free(this->getText);
							free(this->senText);
							exit(0);
						}
					}
					if(logFunc!=NULL)
						logFunc(this->recText(),temp.data.fd);
					if(isLongCon==false)
					{
						epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
						closeSocket(temp.data.fd);
					}
				}
				else
				{
					if(this->clientOut!=NULL)
					{
						int port=0;
						strcpy((char*)this->getText,this->getPeerIp(temp.data.fd,&port));
						clientOut(*this,temp.data.fd,this->getText,port);
					}
					epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
					closeSocket(temp.data.fd);
				}
			}
		}
		return ;
	}
#endif
	RouteFuntion* addRoute()
	{
		RouteFuntion* temp=NULL; 
		if(maxNum-now<=2)
		{
			temp=(RouteFuntion*)realloc(arrRoute,sizeof(RouteFuntion)*(maxNum+10));
			if(temp==NULL)
				return NULL;
			arrRoute=temp;
			maxNum+=10;
		}
		temp=arrRoute+now;
		now++;
		return temp;
	}
	struct ThreadArg{
		HttpServer* pserver;
		int soc;
	};
	void threadHttp()
	{
		while(this->isContinue)
		{
			sockaddr_in newaddr={0,0,{0},{0}};
			int newClient=acceptSocket(newaddr);
			if(newClient==-1)
				continue;
			ThreadArg* temp=new ThreadArg;
			temp->pserver=this;
			temp->soc=newClient;
			ThreadPool::Task task={threadWorker,temp};
			pool->addTask(task);
		}
	}
	static void* threadWorker(void* self)
	{
		ThreadArg* argv=(ThreadArg*)self;
		HttpServer& server=*argv->pserver;
		int cli=argv->soc;
		char* rec=(char*)malloc(sizeof(char)*server.recLen);
		unsigned int size=sizeof(char)*server.recLen;
		if(rec==NULL)
		{
			server.closeSocket(cli);
			free(self);
			return NULL;
		}
		memset(rec,0,sizeof(char)*server.recLen);
		if(server.clientIn!=NULL)
		{
			int port=0;
			server.getPeerIp(cli,&port);
			server.clientIn(server,cli,server.senText,port);
		}
		int recLen=server.receiveSocket(cli,rec,size);
		int all=recLen;
		while((int)size==all)
		{
			rec=(char*)server.enlargeMemory(rec,size);
			recLen=server.receiveSocket(cli,rec,size);
			if(recLen<=0)
				break;
			all+=recLen;
		}
		while(all>=10)
		{
			server.pool->mutexLock();
			server.textLen=all;
			server.getText=rec;
			server.func(cli);
			if(server.logFunc!=NULL)
				server.logFunc(rec,cli);
			server.getText=NULL;
			server.pool->mutexUnlock();
			if(server.isLongCon==false)
				break;
			memset(rec,0,sizeof(char)*server.recLen);
			all=server.receiveSocket(cli,rec,size);
		}
		if(server.clientOut!=NULL)
		{
			int port=0;
			strcpy((char*)server.senText,server.getPeerIp(cli,&port));
			server.clientOut(server,cli,server.senText,port);
		}
		free(rec);
		free(self);
		server.closeSocket(cli);
		return NULL;
	}
	static void loadFile(HttpServer& server,DealHttp& http,int)
	{
		int len=0,flag=0;
		char ask[200]={0},buf[500]={0},temp[200]={0};
		http.getAskRoute(server.recText(),"GET",ask,200);
		HttpServer::RouteFuntion& route=*server.getNowRoute();
		http.getWildUrl(ask,route.route,temp,200);
		sprintf(buf,"GET %s%s HTTP/1.1",route.pathExtra,temp);
		do
		{
			flag=http.autoAnalysisGet(buf,(char*)server.getSenBuffer(),server.senLen,NULL,&len);
			if(flag==2&&server.senLen<server.maxLen*1024*1024)
				server.enlagerSenBuffer();
			else if(flag==1)
			{
				if(flag==1&&server.logError!=NULL)
					server.logError(server.error,0);
				if(server.isDebug)
					printf("404 get %s wrong\n",buf);
			}
			else
				break;
		}while(flag==2);
		staticLen(len);
	}
	static void deleteFile(HttpServer& server,DealHttp& http,int senLen)
	{
		int len=0;
		http.customizeAddTop(server.getSenBuffer(),senLen,DealHttp::STATUSFORBIDDEN,strlen("403 forbidden"),"text/plain");
		http.customizeAddBody(server.getSenBuffer(),senLen,"403 forbidden",strlen("403 forbidden"));
		len=strlen((char*)server.getSenBuffer());
		staticLen(len);
	}
	static void rediectGram(HttpServer& server,DealHttp& http,int)
	{
		auto now=server.getNowRoute();
		http.gram.statusCode=DealHttp::STATUSMOVTEMP;
		http.gram.head.insert(std::pair<std::string,std::string>{"Location",now->pathExtra});
	}
	static unsigned staticLen(int senLen)
	{
		static unsigned len=0;
		if(senLen<0)
			return len;
		else
			len=senLen;
		return 0;
	}
	void* enlargeMemory(void* old,unsigned& oldSize)
	{
		oldSize*=2;
		void* temp=realloc(old,oldSize);
		if(temp==NULL)
		{
			error="server:malloc wrong";
			if(logError!=NULL)
				logError(error,0);
			oldSize/=2;
			return old;
		}
		else
			return temp;
	}
#ifndef _WIN32
	static void sigCliDeal(int )
	{
		while(waitpid(-1, NULL, WNOHANG)>0);
	}
#endif
};
/*******************************
 * author:chenxuan
 * class:the same as servertcpip but it use threadpool
 * example:see ServerTcpIp
******************************/
class ServerPool:public ServerTcpIp{
private:
	struct Argv{
		ServerPool* pserver;
		void (*func)(ServerPool&,int);
		int soc;
		Argv()
		{
			pserver=NULL;
			soc=-1;
			func=NULL;
		}
	};
private:
	ThreadPool* pool;
	pthread_mutex_t mutex;
	unsigned int threadNum;
	bool isEpoll;
private:
#ifndef _WIN32
	static void sigCliDeal(int )
	{
		while(waitpid(-1, NULL, WNOHANG)>0);
	}
#endif
	static void* worker(void* argc)
	{
		Argv argv=*(Argv*)argc;
		delete (Argv*)argc;
		if(argv.func!=NULL)
			argv.func(*argv.pserver,argv.soc);
		return NULL;
	}
public:
	ServerPool(unsigned short port,unsigned int threadNum=0):ServerTcpIp(port)
	{
		this->threadNum=threadNum;
		if(threadNum>0)
		{	
			pool=new ThreadPool(threadNum);
			if(pool==NULL)
			{
				this->error="malloc wrong";
				return;
			}
		}
		pthread_mutex_init(&mutex,NULL);
		isEpoll=false;
	}
	~ServerPool()
	{
		delete pool;
	   	pthread_mutex_destroy(&mutex);
	}
	inline void mutexLock()
	{
		pthread_mutex_lock(&mutex);
	}
	inline void mutexUnlock()
	{
		pthread_mutex_unlock(&mutex);
	}
	bool mutexTryLock()
	{
		if(0==pthread_mutex_trylock(&mutex))
			return true;
		else
			return false;
	}
	void threadModel(void (*pfunc)(ServerPool&,int))
	{
		if(this->threadNum==0)
		{
			this->error="thread wrong init";
			return;
		}
		while(1)
		{
			sockaddr_in newaddr={0,0,{0},{0}};
			int newClient=acceptSocket(newaddr);
			if(newClient==-1)
				continue;
			Argv* temp=new Argv;
			if(temp==NULL)
			{
				error="malloc wrong";
				return;
			}
			temp->pserver=this;
			temp->func=pfunc;
			temp->soc=newClient;
			ThreadPool::Task task={worker,temp};
			pool->addTask(task);
		}
	}
#ifndef _WIN32
	void forkModel(void* pneed,void (*pfunc)(ServerPool&,int,void*))
	{
		signal(SIGCHLD,sigCliDeal);
		while(1)
		{
			sockaddr_in newaddr={0,0,{0},{0}};
			int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
			if(newClient==-1)
				continue;
			if(fork()==0)
			{
				close(sock);
				pfunc(*this,newClient,pneed);
			}
			close(newClient);
		}
	}
	void forkEpoll(unsigned int senBufChar,unsigned int recBufChar,void (*pfunc)(ServerPool::Thing,int,int,void*,void*,ServerPool&))
	{
		signal(SIGCHLD,sigCliDeal);
		char* pneed=(char*)malloc(sizeof(char)*senBufChar),*pget=(char*)malloc(sizeof(char)*recBufChar);
		if(pneed==NULL||pget==NULL)
		{
			this->error="malloc worng";
			return;
		}
		memset(pneed,0,sizeof(char)*senBufChar);
		memset(pget,0,sizeof(char)*recBufChar);
		while(1)
		{
			int eventNum=epoll_wait(epfd,pevent,512,-1),thing=0;
			for(int i=0;i<eventNum;i++)
			{
				epoll_event temp=pevent[i];
				if(temp.data.fd==sock)
				{
					sockaddr_in newaddr={0,0,{0},{0}};
					int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
					nowEvent.data.fd=newClient;
					nowEvent.events=EPOLLIN;
					epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
					strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
				}
				else
				{
					memset(pget,0,sizeof(char)*recBufChar);
					int getNum=recv(temp.data.fd,(char*)pget,recBufChar,0);
					if(getNum>0)
						thing=2;
					else
					{
						*(char*)pget=0;
						thing=0;
						epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
						close(temp.data.fd);
					}
					if(pfunc!=NULL&&thing==2)
					{
						if(fork()==0)
						{
							close(sock);
							pfunc(CPPSAY,temp.data.fd,getNum,pget,pneed,*this);
							close(temp.data.fd);
							free(pneed);
							free(pget);
							exit(0);
						}
						else
						{
							epoll_ctl(epfd,temp.data.fd,EPOLL_CTL_DEL,NULL);
							close(temp.data.fd);
						}
					}
				}
			}
		}
	}
	void epollThread(void (*pfunc)(ServerPool&,int))
	{
		if(this->threadNum==0)
		{
			this->error="thread wrong init";
			return;
		}
		isEpoll=true;
		int eventNum=epoll_wait(epfd,pevent,512,-1);
		for(int i=0;i<eventNum;i++)
		{
			epoll_event temp=pevent[i];
			if(temp.data.fd==sock)
			{
				sockaddr_in newaddr={0,0,{0},{0}};
				int newClient=accept(sock,(sockaddr*)&newaddr,(socklen_t*)&sizeAddr);
				nowEvent.data.fd=newClient;
				nowEvent.events=EPOLLIN|EPOLLET;
				epoll_ctl(epfd,EPOLL_CTL_ADD,newClient,&nowEvent);
			}
			else
			{
				if(pfunc!=NULL)
				{
					Argv* argv=new Argv;
					argv->func=pfunc;
					argv->soc=temp.data.fd;
					argv->pserver=this;
					ThreadPool::Task task={worker,argv};
					pool->addTask(task);
				}
			}
		}
		return;
	}
#endif
	inline void threadDeleteSoc(int clisoc)
	{
		closeSocket(clisoc);
#ifndef _WIN32
		if(isEpoll)
			epoll_ctl(epfd,clisoc,EPOLL_CTL_DEL,NULL);
#endif
	}
};
/*******************************
 * author:chenxuan
 * class:easy to get file buffer and write to file
 * example:{
 * FileGet file;
 * const char* buffer=file.getFileBuff("file name");
 * FileGet::writeToFile(buffer);
 * }
******************************/
class FileGet{
private:
	char* pbuffer;
public:
	FileGet()
	{
		pbuffer=NULL;
	}
	~FileGet()
	{
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
	}
	static int getFileLen(const char* fileName)
	{
		int len=0;
		FILE* fp=fopen(fileName,"rb");
		if(fp==NULL)
			return -1;
		fseek(fp,0,SEEK_END);
		len=ftell(fp);
		fclose(fp);
		return len;
	}
	static bool getFileMsg(const char* fileName,char* buffer,unsigned int bufferLen)
	{
		unsigned int i=0,len=0;
		len=getFileLen(fileName);
		FILE* fp=fopen(fileName,"rb");
		if(fp==NULL)
			return false;
		for(i=0;i<len&&i<bufferLen;i++)
			buffer[i]=fgetc(fp);
		buffer[i+1]=0;
		fclose(fp);
		return true;
	}
	const char* getFileBuff(const char* fileName)
	{
		if(pbuffer!=NULL)
			free(pbuffer);
		int len=getFileLen(fileName);
		if(len==-1)
			return NULL;
		pbuffer=(char*)malloc(sizeof(char)*getFileLen(fileName)+10);
		if(pbuffer==NULL)
			return NULL;
		getFileMsg(fileName,pbuffer,sizeof(char)*getFileLen(fileName)+10);
		return pbuffer;
	}
	static bool writeToFile(const char* fileName,const char* buffer,unsigned int writeLen,bool isCat=false)
	{
		FILE* fp=NULL;
		if(!isCat)
			fp=fopen(fileName,"wb+");
		else
			fp=fopen(fileName,"ab+");
		if(fp==NULL)
			return false;
		for(unsigned int i=0;i<writeLen;i++)
			fputc(buffer[i],fp);
		fclose(fp);
		return true;
	}
};
}
#endif
