#ifndef _CPPWEB_H_
#define _CPPWEB_H_
#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<time.h>
#include<iostream>
#include<string>
#include<winsock2.h>
#include<pthread.h>
#include<queue>
#include<vector>
#include<stack>
#include<string>
#include<unordered_map>
namespace cppweb{

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
};
class Json{
public:
	enum TypeJson{
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
		float floVal;
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
	char* text;
	const char* error;
	unsigned maxLen;
	unsigned floNum;
	Object* obj;
	std::unordered_map<char*,unsigned> memory;
	std::unordered_map<std::string,Object*> hashMap;
	std::unordered_map<char*,char*> bracket;
public:
	Json()
	{
		error=NULL;
		obj=NULL;
		text=NULL;
		maxLen=256;
		floNum=3;
	}
	Json(const char* jsonText)
	{
		error=NULL;
		obj=NULL;
		maxLen=256;
		floNum=3;
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
		if(text!=NULL)
			free(text);
		for(auto iter=memory.begin();iter!=memory.end();iter++)
			free(iter->first);
	}
	const char* formatPrint(const Object* exmaple,unsigned buffLen)
	{
		char* buffer=(char*)malloc(sizeof(char)*buffLen);
		if(buffer==NULL)
		{
			error="malloc wrong";
			return NULL;
		}
		memset(buffer,0,sizeof(char)*buffLen);
		memory.insert(std::pair<char*,unsigned>{buffer,buffLen});
		printObj(buffer,exmaple);
		return buffer;
	}
	Object* operator[](const char* key)
	{
		if(hashMap.find(std::string(key))==hashMap.end())
			return NULL;
		return hashMap.find(std::string(key))->second;
	}
	bool addKeyVal(char* obj,TypeJson type,const char* key,...)
	{
		if(obj==NULL)
		{
			error="null buffer";
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
		if(memory[obj]-strlen(obj)<strlen(key)+4)
		{
			error="obj too short";
			return false;
		}
		sprintf(obj,"%s\"%s\":",obj,key);
		int valInt=0;
		char* valStr=NULL;
		float valFlo=9;
		bool valBool=false;
		switch(type)
		{
		case INT:
			valInt=va_arg(args,int);
			if(memory[obj]-strlen(obj)<15)
			{
				error="obj too short";
				return false;
			}
			sprintf(obj,"%s%d",obj,valInt);
			break;
		case FLOAT:
			valFlo=va_arg(args,double);
			if(memory[obj]-strlen(obj)<15)
			{
				error="obj too short";
				return false;
			}
			sprintf(obj,"%s%.*f",obj,floNum,valFlo);
			break;
		case STRING:
			valStr=va_arg(args,char*);
			if(valStr==NULL)
			{
				error="null input";
				return false;
			}
			if(memory[obj]-strlen(obj)<strlen(obj)+5)
			{
				error="obj too short";
				return false;
			}
			sprintf(obj,"%s\"%s\"",obj,valStr);
			break;
		case EMPTY:
			if(memory[obj]-strlen(obj)<5)
			{
				error="obj too short";
				return false;
			}
			strcat(obj,"null");
			break;
		case BOOL:
			valBool=va_arg(args,int);
			if(memory[obj]-strlen(obj)<5)
			{
				error="obj too short";
				return false;
			}
			if(valBool==true)
				strcat(obj,"true");
			else
				strcat(obj,"false");
			break;
		case OBJ:
		case ARRAY:
			valStr=va_arg(args,char*);
			if(memory[obj]-strlen(obj)<strlen(obj)+5)
			{
				error="obj too short";
				return false;
			}
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
	char* createObject(unsigned maxBuffLen)
	{
		char* now=(char*)malloc(sizeof(char)*maxBuffLen);
		if(now==NULL||maxBuffLen<4)
		{
			error="init worng";
			return NULL;
		}
		else
			memory.insert(std::pair<char*,int>{now,maxBuffLen});
		memset(now,0,sizeof(char)*maxBuffLen);
		strcpy(now,"{}");
		return now;
	}
	char* createArray(unsigned maxBuffLen,TypeJson type,unsigned arrLen,void* arr)
	{
		if(arr==NULL||maxBuffLen<4)
		{
			error="null input";
			return NULL;
		}
		char* now=(char*)malloc(sizeof(char)*maxBuffLen);
		if(now==NULL)
		{
			error="malloc worng";
			return NULL;
		}
		else
			memory.insert(std::pair<char*,int>{now,maxBuffLen});
		memset(now,0,sizeof(char)*maxBuffLen);
		strcat(now,"[");
		int* arrInt=(int*)arr;
		float* arrFlo=(float*)arr;
		char** arrStr=(char**)arr;
		bool* arrBool=(bool*)arr;
		unsigned i=0;
		switch(type)
		{
		case INT:
			for(i=0;i<arrLen;i++)
			{
				if(maxBuffLen-strlen(now)<std::to_string(arrInt[i]).size()+3)
				{
					error="bufferLen is too small";
					return NULL;
				}
				sprintf(now,"%s%d,",now,arrInt[i]);
			}
			break;
		case FLOAT:
			for(i=0;i<arrLen;i++)
			{
				if(maxBuffLen-strlen(now)<std::to_string(arrFlo[i]).size()+3)
				{
					error="bufferLen is too small";
					return NULL;
				}
				sprintf(now,"%s%.*f,",now,floNum,arrFlo[i]);
			}
			break;
		case STRING:
			for(i=0;i<arrLen;i++)
			{
				if(maxBuffLen-strlen(now)<strlen(arrStr[i])+5)
				{
					error="bufferLen is too small";
					return NULL;
				}
				sprintf(now,"%s\"%s\",",now,arrStr[i]);
			}
			break;
		case OBJ:
		case ARRAY:
			for(i=0;i<arrLen;i++)
			{
				if(maxBuffLen-strlen(now)<strlen(arrStr[i])+4)
				{
					error="bufferLen is too small";
					return NULL;
				}
				sprintf(now,"%s%s,",now,arrStr[i]);
			}
			break;
		case BOOL:
			for(i=0;i<arrLen;i++)
			{
				if(maxBuffLen-strlen(now)<6)
				{
					error="bufferLen is too small";
					return NULL;
				}
				if(arrBool)
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
	inline Object* getRootObj()
	{
		return obj;
	}
	inline const char* lastError()
	{
		return error;
	}
	inline void changeSetting(unsigned keyValMaxLen,unsigned floNum)
	{
		this->maxLen=keyValMaxLen>maxLen?keyValMaxLen:maxLen;
		this->floNum=floNum;
	}
private:
	Object* analyseObj(char* begin,char* end)
	{
		Object * root=new Object,*last=root;
		root->type=STRUCT;
		char* now=begin+1,*next=now;
		char* word=(char*)malloc(sizeof(char)*maxLen),*val=(char*)malloc(sizeof(char)*maxLen),temp=*end;
		if(word==NULL||val==NULL)
		{
			error="malloc wrong";
			return NULL;
		}
		memset(word,0,sizeof(char)*maxLen);
		memset(val,0,sizeof(char)*maxLen);
		*end=0;
		while(now<end)
		{
			Object* nextObj=new Object;
			memset(word,0,sizeof(char)*maxLen);
			findString(now,word,maxLen);
			nextObj->key=word;
			hashMap.insert(std::pair<std::string,Object*>{word,nextObj});
			now+=strlen(word)+3;
			if(*now=='\"')
			{
				nextObj->type=STRING;
				next=strchr(now+1,'\"');
				while(*(next-1)=='\\')
					next=strchr(next+1,'\"');
				if(next==NULL)
				{
					error="string wrong";
					return NULL;
				}
				for(unsigned i=0;now+i+1<next;i++)
					val[i]=*(now+i+1);
				val[strlen(val)]=0;
				nextObj->strVal=val;
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
					sscanf(now,"%f",&nextObj->floVal);
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
				free(word);
				free(val);
				return root;
			}
			last->nextObj=nextObj;
			last=nextObj;
		}
		*end=temp;
		free(word);
		free(val);
		return root;
	}
	TypeJson analyseArray(char* begin,char* end,std::vector<Object*>& array)
	{
		char* now=begin+1,*next=end,*word=(char*)malloc(sizeof(char)*maxLen);
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
					array.push_back(nextObj);
				}
				else
				{
					findNum(now,type,&nextObj->floVal);
					array.push_back(nextObj);
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
				array.push_back(nextObj);
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
				array.push_back(nextObj);
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
				array.push_back(nextObj);
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
				array.push_back(nextObj);
				now=next;
				now=strchr(now+1,',');
				if(now==NULL)
					break;
				now+=1;
			}
		}
		else if(*now==']')
		{
			free(word);
			return INT;
		}
		else
		{
			error="array find wrong";
			free(word);
			return INT;
		}
		free(word);
		return nextObj->type;
	}
	void findString(const char* begin,char* buffer,unsigned buffLen)
	{
		const char* now=begin+1,*next=now;
		next=strchr(now+1,'\"');
		while(*(next-1)=='\\')
			next=strchr(next+1,'\"');
		for(unsigned i=0;now+i<next&&i<buffLen;i++)
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
			if(sscanf(begin,"%f",(float*)pnum)<1)
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
			{
				while(text[i]!='\n'&&i<strlen(text))
				{
					text[i]=' ';
					i++;
				}
			}
			else if(flag%2==0&&text[i]=='/'&&i+1<strlen(text)&&text[i+1]=='*')
			{
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
			else if(text[i]==']'||text[i]=='}')
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
	bool printObj(char* buffer,const Object* obj)
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
			for(unsigned i=0;i<deep+4;i++)
				strcat(buffer," ");
			switch(now->type)
			{
			case INT:
				sprintf(buffer,"%s\"%s\":%d,",buffer,now->key.c_str(),now->intVal);
				break;
			case FLOAT:
				sprintf(buffer,"%s\"%s\":%.*f,",buffer,now->key.c_str(),floNum,now->floVal);
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
	bool printArr(char* buffer,TypeJson type,const std::vector<Object*>& arr)
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
			switch(type)
			{
			case INT:
				sprintf(buffer,"%s%d,",buffer,arr[i]->intVal);
				break;
			case FLOAT:
				sprintf(buffer,"%s%.*f,",buffer,floNum,arr[i]->floVal);
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
class ThreadPool{
public://a struct for you to add task
	struct Task{
		void* (*ptask)(void*);
		void* arg;
	};
private:
	std::queue<Task> thingWork;//a queue for struct task
	pthread_cond_t condition;//a condition mutex
	pthread_mutex_t lockPoll;//a lock to lock queue
	pthread_mutex_t lockTask;//a lock for user to ctrl
	pthread_mutex_t lockBusy;//a lock for busy thread
	pthread_t* thread;//an array for thread
	pthread_t threadManager;//thread for manager user to ctrl
	unsigned int liveThread;//num for live thread
	unsigned int busyThread;//num for busy thread
	bool isContinue;//if the pool is continue
private:
	static void* worker(void* arg)//the worker for user
	{
		ThreadPool* poll=(ThreadPool*)arg;
		while(1)
		{
			pthread_mutex_lock(&poll->lockPoll);
			while(poll->isContinue==true&&poll->thingWork.size()==0)
				pthread_cond_wait(&poll->condition,&poll->lockPoll);
			if(poll->isContinue==false)
			{
				pthread_mutex_unlock(&poll->lockPoll);
				pthread_exit(NULL);
			}
			if(poll->thingWork.size()>0)
			{
				pthread_mutex_lock(&poll->lockBusy);
				poll->busyThread++;
				pthread_mutex_unlock(&poll->lockBusy);
				ThreadPool::Task task=poll->thingWork.front();
				poll->thingWork.pop();
				pthread_mutex_unlock(&poll->lockPoll);
				task.ptask(task.arg);
				pthread_mutex_lock(&poll->lockBusy);
				poll->busyThread--;
				pthread_mutex_unlock(&poll->lockBusy);
			}
		}
		return NULL;
	}
	static void* manager(void* )//manager for user
	{
		return NULL;
	}
public:
	ThreadPool(unsigned int threadNum=10)//create threads
	{
		if(threadNum<=1)
			threadNum=10;
		thread=new pthread_t[threadNum];
		if(thread==NULL)
			return ;
		for(unsigned int i=0;i<threadNum;i++)
			thread[i]=0;
		pthread_cond_init(&condition,NULL);
		pthread_mutex_init(&lockPoll,NULL);
		pthread_mutex_init(&lockTask,NULL);
		pthread_mutex_init(&lockBusy,NULL);
		liveThread=threadNum;
		isContinue=true;
		busyThread=0;
		pthread_create(&threadManager,NULL,manager,this);
		for(unsigned int i=0;i<threadNum;i++)
			pthread_create(&thread[i],NULL,worker,this);
	}
	~ThreadPool()//destory pool
	{
		if(isContinue==false)
			return;
		isContinue=false;
		pthread_join(threadManager,NULL);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_cond_signal(&condition);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_join(thread[i],NULL);
		pthread_cond_destroy(&condition);
		pthread_mutex_destroy(&lockPoll);
		pthread_mutex_destroy(&lockTask);
		pthread_mutex_destroy(&lockBusy);
		delete[] thread;
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
		pthread_mutex_lock(&this->lockPoll);
		this->thingWork.push(task);
		pthread_mutex_unlock(&this->lockPoll);
		pthread_cond_signal(&this->condition);
	}
	void endPool()//user delete the pool
	{
		isContinue=false;
		pthread_join(threadManager,NULL);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_cond_signal(&condition);
		for(unsigned int i=0;i<liveThread;i++)
			pthread_join(thread[i],NULL);
		pthread_cond_destroy(&condition);
		pthread_mutex_destroy(&lockPoll);
		pthread_mutex_destroy(&lockTask);
		delete[] thread;
	}
	void getBusyAndTask(unsigned int* pthread,unsigned int* ptask)//get busy live and task num
	{
		pthread_mutex_lock(&lockBusy);
		*pthread=busyThread;
		pthread_mutex_unlock(&lockBusy);
		pthread_mutex_lock(&lockPoll);
		*ptask=thingWork.size();
		pthread_mutex_unlock(&lockPoll);
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
class ServerTcpIp{
protected:
	int sizeAddr;//sizeof(sockaddr_in) connect with addr_in;
	int backwait;//the most waiting clients ;
	int numClient;//how many clients now;
	char* hostip;//host IP
	char* hostname;//host name
	const char* error;//error hapen
	WSADATA wsa;//apply verson of windows;
	SOCKET sock;//file descriptor of host;
	SOCKET sockC;//file descriptor to sign every client;
	SOCKADDR_IN addr;//IPv4 of host;
	SOCKADDR_IN client;//IPv4 of client;
	fd_set  fdClients;//file descriptor
protected:
    int* pfdn;//pointer if file descriptor
    int fdNumNow;//num of fd now
    int fdMax;//fd max num
    bool addFd(int addsoc)
    {
        bool flag=false;
        for(int i=0;i<fdNumNow;i++)
        {
            if(pfdn[i]==0)
            {
                pfdn[i]=addsoc;
                flag=true;
                break;
            }
        }
        if(flag==false)
        {
            if(fdNumNow>=fdMax)
            {
                pfdn=(int*)realloc(pfdn,sizeof(int)*(fdMax+32));
                if(pfdn==NULL)
                    return false;
                fdMax+=31;
            }
            pfdn[fdNumNow]=addsoc;
            fdNumNow++;
        }
        return true;
    }
    bool deleteFd(int clisoc)
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
public:
	ServerTcpIp(unsigned short port=5200,int wait=5)
	{
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
		{
			error="WSA wrong";
			return;
		}
		sock=socket(AF_INET,SOCK_STREAM,0);//AF=addr family internet
		addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);//inaddr_any
		addr.sin_family=AF_INET;//af_intt IPv4
		addr.sin_port=htons(port);//host to net short
		sizeAddr=sizeof(sockaddr);
		backwait=wait;
		numClient=0;
		fdNumNow=0;
		fdMax=100;
		pfdn=(int*)malloc(sizeof(int)*100);
		memset(pfdn,0,sizeof(int)*100);
		hostip=(char*)malloc(sizeof(char)*20);
		memset(hostip,0,sizeof(char)*20);
		hostname=(char*)malloc(sizeof(char)*30);
		memset(hostname,0,sizeof(char)*30);
		FD_ZERO(&fdClients);//clean fdClients;
	}
	~ServerTcpIp()//clean server
	{
		closesocket(sock);
		closesocket(sockC);
		free(pfdn);
		free(hostip);
		free(hostname);
		WSACleanup();
	}
	bool bondhost()//bond myself
	{
		if(bind(sock,(sockaddr*)&addr,sizeof(addr))==-1)
			return false;
		return true;
	}
	bool setlisten()//set listem to accept
	{
		if(listen(sock,backwait)==SOCKET_ERROR)
			return false;
		FD_SET(sock,&fdClients);
		return true;
	}
	unsigned int acceptClient()//wait until success
	{
		sockC=accept(sock,(sockaddr*)&client,&sizeAddr);
		return sockC;
	}
	bool acceptClients(unsigned int* psock)//model two
	{
		*psock=accept(sock,(sockaddr*)&client,&sizeAddr);
		return this->addFd(*psock);
	}
	inline int receiveOne(void* pget,int len)//model one
	{
		return recv(sockC,(char*)pget,len,0);
	}
	inline int receiveSocket(unsigned int sock,void* prec,int len)//model two
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendClientOne(const void* psend,int len)//model one
	{
		return send(sockC,(char*)psend,len,0);
	}
	inline int sendSocket(unsigned int sock ,const void* psend,int len)//model two
	{
		return send(sock,(char*)psend,len,0);
	}
	void sendEverySocket(const void* psen,int len)//model two
	{
		for(int i=0;i<fdNumNow;i++)
			if(pfdn[i]!=0)
			    send(pfdn[i],(char*)psen,len,0);
	}
	bool epollModel(void* pget,int len,void* pneed,int (*pfunc)(int,int ,int ,void* ,void*,ServerTcpIp& ))
	{//0 out,1 in,2 say
		fd_set temp=fdClients;
		int thing=2;
		int num=0;
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
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							thing=1;
							num=newClient;
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(pfunc!=NULL)
							{
								if(pfunc(thing,num,0,pget,pneed,*this))
									return false;
							}
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						num=fdClients.fd_array[i];
						if(sRec>0)
							thing=2;
						if(sRec<=0)
						{
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);
							thing=0;
						}
						if(pfunc!=NULL)
						{
							if(pfunc(thing,num,sRec,pget,pneed,*this))
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
	bool selectModel(void* pget,int len,void* pneed,int (*pfunc)(int,int ,int ,void* ,void*,ServerTcpIp& ))
	{//0 out,1 in,2 say
		fd_set temp=fdClients;
		int thing=2;
		int num=0;
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
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							thing=1;
							num=newClient;
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(pfunc!=NULL)
							{
								if(pfunc(thing,num,0,pget,pneed,*this))
									return false;
							}
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						num=fdClients.fd_array[i];
						if(sRec>0)
							thing=2;
						if(sRec<=0)
						{
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);
							thing=0;
						}
						if(pfunc!=NULL)
						{
							if(pfunc(thing,num,sRec,pget,pneed,*this))
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
//	bool selectSendEveryone(void* psend,int len)
//	{
//		for(unsigned int i=0;i<fdClients.fd_count;i++)
//			if(fdClients.fd_array[i]!=0)
//				send(fdClients.fd_array[i],(char*)psend,len,0);
//		return true;
//	}
//	bool updateSocketSelect(SOCKET* psocket,int* pcount)
//	{
//		if(fdClients.fd_count!=0)
//			*pcount=fdClients.fd_count;
//		else
//			return false;
//		for(unsigned int i=0;i<fdClients.fd_count;i++)
//			psocket[i]=fdClients.fd_array[i];
//		return true;
//	}
//	bool sendSocketSelect(SOCKET toClient,const void* psend,int len)
//	{
//		for(unsigned int i=0;i<fdClients.fd_count;i++)
//		{
//			if(toClient==fdClients.fd_array[i])
//			{
//				send(fdClients.fd_array[i],(char*)psend,len,0);
//				return true;
//			}
//		}
//		return false;
//	}
//	SOCKET findSocketSelect(int i)
//	{
//		if(fdClients.fd_array[i]!=0)
//			return fdClients.fd_array[i];
//		else
//			return -1;
//	}
    bool disconnectSocket(SOCKET clisock)
    {
        for(unsigned int i=0;i<fdClients.fd_count;i++)
            if(fdClients.fd_array[i]==clisock)
                FD_CLR(fdClients.fd_array[i],&fdClients);
        closesocket(clisock);
        return this->deleteFd(clisock);
    }
	bool updateSocket(SOCKET* array,int* pcount,int arrayLen)
	{
		if(fdNumNow!=0)
			*pcount=fdNumNow;
		else
			return false;
		for(int i=0;i<fdNumNow&&i<arrayLen;i++)
			array[i]=pfdn[i];
		return true;
	}
	bool findSocket(int cliSoc)
	{
		for(int i=0;i<fdNumNow;i++)
			if(pfdn[i]==cliSoc)
				return true;
		return false;
	}
	char* getHostIp()
	{
		char name[30]={0};
		gethostname(name,30);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.S_un.S_addr,p,phost->h_length);
		memset(hostip,0,sizeof(char)*20);
		memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return hostip;
	}
	char* getHostName()
	{
		char name[30]={0};
		gethostname(name,30);
		memcpy(hostname,name,30);
		return hostname;
	}
	char* getPeerIp(SOCKET cliSoc,int* pcliPort)
	{
		SOCKADDR_IN cliAddr={0,0,{0,0,0,0},0};
		int len=sizeof(cliAddr);
		if(-1==getpeername(cliSoc,(SOCKADDR*)&cliAddr,&len))
			return NULL;
		*pcliPort=cliAddr.sin_port;
		return inet_ntoa(cliAddr.sin_addr);
	}
};
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
			SOCKADDR_IN newaddr={0,0,{0},{0}};
			int newClient=accept(sock,(SOCKADDR*)&newaddr,&sizeAddr);
			if(newClient==-1)
				continue;
			this->addFd(newClient);
			Argv* temp=new Argv;
			temp->pserver=this;
			temp->func=pfunc;
			temp->soc=newClient;
			ThreadPool::Task task={worker,temp};
			pool->addTask(task);
		}
	}
	inline bool threadDeleteSoc(int clisoc)
	{
		closesocket(clisoc);
		return this->deleteFd(clisoc);
	}
};
class ClientTcpIp{
private:
	WSADATA wsa;//apply for api
	SOCKET sock;//myself
	SOCKADDR_IN addrC;//server information
	char ip[20];//server Ip
	char sen[200];//what you send
	char rec[200];//what you get
	char* hostip;//host ip
	char* hostname;//host name
//	SSL* ssl;
//	SSL_CTX* ctx;
public:
	ClientTcpIp(const char* hostIp,int port=5200)
	{
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
			exit(0);
		memset(ip,0,20);
		if(hostIp!=NULL)
			strcpy(ip,hostIp);
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(hostIp!=NULL)
			addrC.sin_addr.S_un.S_addr=inet_addr(ip);
		addrC.sin_family=AF_INET;
		addrC.sin_port=htons(port);
		hostip=(char*)malloc(sizeof(char)*20);
		memset(hostip,0,sizeof(char)*20);
		hostname=(char*)malloc(sizeof(char)*30);
		memset(hostname,0,sizeof(char)*30);
//		ssl=NULL;
//		ctx=NULL;
	}
	~ClientTcpIp()
	{
//		if(ssl!=NULL)
//		{
//			SSL_shutdown(ssl);
//			SSL_free(ssl);
//		}
//		if(ctx!=NULL)
//			SSL_CTX_free(ctx);
		free(hostip);
		free(hostname);
		closesocket(sock);
		WSACleanup();
	}
	void addHostIp(const char* ip,unsigned short port=0)
	{
		if(ip==NULL)
			return;
		strcpy(this->ip,ip);
		addrC.sin_addr.S_un.S_addr=inet_addr(ip);
		if(port!=0)
			addrC.sin_port=htons(port);
	}
	bool tryConnect()
	{
		if(connect(sock,(SOCKADDR*)&addrC,sizeof(sockaddr))==SOCKET_ERROR)
			return false;
		return true;
	}
	inline int receiveHost(void* prec,int len)
	{
		return recv(sock,(char*)prec,len,0);
	}
	inline int sendHost(const void* psend,int len)
	{
		return send(sock,(char*)psend,len,0);
	}
	bool disconnectHost()
	{
		closesocket(sock);
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(sock<=0)
			return false;
		return true;
	}
	char* getSelfIp()
	{
		char name[30]={0};
		gethostname(name,30);
		hostent* phost=gethostbyname(name);
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.S_un.S_addr,p,phost->h_length);
		memset(hostip,0,sizeof(char)*20);
		memcpy(hostip,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		return hostip;
	}
	char* getSelfName()
	{
		char name[30]={0};
		gethostname(name,30);
		memcpy(hostname,name,30);
		return hostname;
	}
	static bool getDnsIp(const char* name,char* ip,unsigned int ipMaxLen)
	{
		WSADATA wsa;
		if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
			return false;
		hostent* phost=gethostbyname(name);
		if(phost==NULL)
			return false;
		in_addr addr;
		char* p=phost->h_addr_list[0];
		memcpy(&addr.S_un.S_addr,p,phost->h_length);
		if(strlen(inet_ntoa(addr))>=ipMaxLen)
			return false;
		strcpy(ip,inet_ntoa(addr));
		return true;
	}
};
class DealHttp{
public:
	enum FileKind{
		UNKNOWN=0,HTML=1,EXE=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP=7,JSON=8,
	};
	enum Status{
		STATUSOK=200,STATUSNOCON=204,STATUSMOVED=301,STATUSBADREQUEST=400,STATUSFORBIDDEN=403,
		STATUSNOFOUND=404,STATUSNOIMPLEMENT=501,
	};
	struct Datagram{
		Status statusCode;
		FileKind typeFile;
		unsigned fileLen;
		std::unordered_map<std::string,std::string> head;
		std::unordered_map<std::string,std::string> cookie;
		const void* body;
	};
	struct Request{
		std::string method;
		std::string askPath;
		std::string version;
	};
private:
	char ask[256];
	char* pfind;
	const char* error;
public:
	DealHttp()
	{
		for(int i=0;i<256;i++)
			ask[i]=0;
		pfind=NULL;
		error=NULL;
	}
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
		return this->findBackString(pfind,strlen(pneed),ask,256);
	}
	inline char* findFirst(void* message,const char* ptofind)
	{
		return strstr((char*)message,ptofind);
	}
	void getRequestMsg(void* message,Request& request)
	{
		char method[30]={0},path[256]={0},version[30]={0};
		sscanf((char*)message,"%30s%256s%30[^\r]",method,path,version);
		request.askPath=path;
		request.method=method;
		request.version=version;
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
		strcat((char*)buffer,"\r\n");
		unsigned int i=0;
		topLen=strlen((char*)buffer);
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
	void createTop(FileKind kind,char* ptop,unsigned int bufLen,int* topLen,unsigned int fileLen)//1:http 2:down 3:pic
	{
		if(bufLen<100)
		{
			this->error="buffer too short";
			return;
		}
		switch (kind)
		{
		   case UNKNOWN:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length:%d\r\n\r\n",fileLen);
			break;
		   case HTML:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:text/html\r\n"
			"Content-Length:%d\r\n\r\n",fileLen);
			break;
		   case EXE:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:application/octet-stream\r\n"
			"Content-Length:%d\r\n\r\n",fileLen);
			break;
		   case IMAGE:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:image\r\n"
			"Content-Length:%d\r\n\r\n",fileLen);
			break;
		   case NOFOUND:
			*topLen=sprintf(ptop,"HTTP/1.1 404 Not Found\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length:%d\r\n\r\n"
			"404 no found",(int)strlen("404 no found"));
			break;
		   case CSS:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:text/css\r\n"
			"Content-Length:%d\r\n\r\n",fileLen);
			break;
		   case JS:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:text/javascript\r\n"
			"Content-Length:%d\r\n\r\n",fileLen);
			break;
		   case ZIP:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:application/zip\r\n"
			"Content-Length:%d\r\n\r\n",fileLen);
			break;
		   case JSON:
			*topLen=sprintf(ptop,"HTTP/1.1 200 OK\r\n"
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n"
			"Content-Type:application/json\r\n"
			"Content-Length:%d\r\n\r\n",fileLen);
			break;
		}
	}
	bool createSendMsg(FileKind kind,char* buffer,unsigned int bufferLen,const char* pfile,int* plong)
	{
		int temp=0;
		int len=0,noUse=0;
		if(kind==NOFOUND)
		{
			this->createTop(kind,buffer,bufferLen,&temp,len);
			*plong=len+temp+1;
			return true;
		}
		len=this->getFileLen(pfile);
		if(len==0)
			return false;
		this->createTop(kind,buffer,bufferLen,&temp,len);
		this->findFileMsg(pfile,&noUse,buffer+temp,bufferLen);
		*plong=len+temp+1;
		return true;
	}
	char* findFileMsg(const char* pname,int* plen,char* buffer,unsigned int bufferLen)
	{
		FILE* fp=fopen(pname,"rb+");
		unsigned int flen=0,i=0;
		if(fp==NULL)
			return NULL;
		fseek(fp,0,SEEK_END);
		flen=ftell(fp);
		if(flen>=bufferLen)
		{
			this->error="buffer too short";
			fclose(fp);
			return NULL;
		}
		fseek(fp,0,SEEK_SET);
		for(i=0;i<flen;i++)
			buffer[i]=fgetc(fp);
		buffer[i]=0;
		*plen=flen;
		fclose(fp);
		return buffer;
	}
	int getFileLen(const char* pname)
	{
		FILE* fp=fopen(pname,"r+");
		int len=0;
		if(fp==NULL)
			return 0;
		fseek(fp,0,SEEK_END);
		len=ftell(fp);
		fclose(fp);
		return len;
	}
	int autoAnalysisGet(const char* message,char* psend,unsigned int bufferLen,const char* pfirstFile,int* plen)
	{
		if(NULL==this->analysisHttpAsk((void*)message))
			return 0;
		if(strcmp(ask,"HTTP/1.1")==0||strcmp(ask,"HTTP/1.0")==0)
		{
			if(false==this->createSendMsg(HTML,psend,bufferLen,pfirstFile,plen))
			{
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			}
			else
				return 1;
		}
		else if(strstr(ask,".html"))
		{
			if(false==this->createSendMsg(HTML,psend,bufferLen,ask,plen))
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			else
				return 1;
		}
		else if(strstr(ask,".exe"))
		{
			if(false==this->createSendMsg(EXE,psend,bufferLen,ask,plen))
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			else
				return 1;
		}
		else if(strstr(ask,".zip"))
		{
			if(false==this->createSendMsg(ZIP,psend,bufferLen,ask,plen))
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			else
				return 1;
		}
		else if(strstr(ask,".png")||strstr(ask,".PNG")||strstr(ask,".jpg")||strstr(ask,".jpeg"))
		{
			if(false==this->createSendMsg(IMAGE,psend,bufferLen,ask,plen))
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			else
				return 1;
		}
		else if(strstr(ask,".css"))
		{
			if(false==this->createSendMsg(CSS,psend,bufferLen,ask,plen))
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			else
				return 1;
		}
		else if(strstr(ask,".js"))
		{
			if(false==this->createSendMsg(JS,psend,bufferLen,ask,plen))
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			else
				return 1;
		}
		else if(strstr(ask,".json"))
		{
			if(false==this->createSendMsg(JSON,psend,bufferLen,ask,plen))
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			else
				return 1;
		}
		else
		{
			if(false==this->createSendMsg(UNKNOWN,psend,bufferLen,ask,plen))
				if(false==this->createSendMsg(NOFOUND,psend,bufferLen,pfirstFile,plen))
					return 0;
				else
					return 2;
			else
				return 1;
		}
		return 1;
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
	const char* getRouteValue(const void* routeMsg,const char* key,char* value,unsigned int valueLen)
	{
		char* temp=strstr((char*)routeMsg,key);
		if(temp==NULL)
			return NULL;
		return this->findBackString(temp,strlen(key),value,valueLen);
	}
	const char* getWildUrl(const void* getText,const char* route,char* buffer,unsigned int maxLen)
	{
		char* temp=strstr((char*)getText,route);
		if(temp==NULL)
			return NULL;
		temp+=strlen(route);
		char format[20]={0};
		sprintf(format,"%%%us",maxLen);
		sscanf(temp,format,buffer);
		return buffer;
	}
	int getRecFile(const void* message,char* fileName,int nameLen,char* buffer,unsigned int bufferLen)
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
		if((end=strstr(top+strlen(buffer),buffer))==NULL)
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
			"Server LCserver/1.1\r\n"
			"Connection: keep-alive\r\n",
			gram.statusCode,statusEng);
		if(gram.fileLen==0)
		{
			sprintf((char*)buffer,"\r\n");
			return strlen((char*)buffer);
		}
		switch(gram.typeFile)
		{
		case UNKNOWN:
		case NOFOUND:
			strcat((char*)buffer,"\r\n");
			return strlen((char*)buffer);
		case HTML:
			sprintf(temp,"Content-Type:%s\r\n","text/html");
			break;
		case EXE:
			sprintf(temp,"Content-Type:%s\r\n","application/octet-stream");
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
		if(gram.head.size()!=0)
			for(auto iter=gram.head.begin();iter!=gram.head.end();iter++)
				customizeAddHead(buffer,bufferLen,iter->first.c_str(),iter->second.c_str());
		if(gram.cookie.size()!=0)
			for(auto iter=gram.cookie.begin();iter!=gram.cookie.end();iter++)
				setCookie(buffer,bufferLen,iter->first.c_str(),iter->second.c_str());
		return customizeAddBody(buffer,bufferLen,(char*)gram.body,gram.fileLen);
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
};
class WebToken{
private:
	char* backString;
	char err[30];
public:
	WebToken()
	{
		backString=NULL;
		memset(err,0,sizeof(char)*30);
		sprintf(err,"no error");
	}
	const char* createToken(const char* key,const char* encryption,char* getString,unsigned int stringLen,unsigned int liveSecond)
	{
		int temp=0;
		if(key==NULL||encryption==NULL||getString==NULL||stringLen<sizeof(char)*strlen(encryption)+30)
		{
			sprintf(err,"input wrong");
			return NULL;
		}
		if(backString!=NULL)
			free(backString);
		backString=(char*)malloc(sizeof(char)*strlen(encryption)+30);
		memset(backString,0,sizeof(char)*strlen(encryption)+30);
		if(backString==NULL)
		{
			sprintf(err,"malloc wrong");
			return NULL;
		}
		for(unsigned int i=0;i<strlen(key);i++)
			temp+=key[i];
		int end=time(NULL)+liveSecond+temp;
		temp=temp%4;
		for(unsigned int i=0;i<strlen(encryption);i++)
		{
			backString[i]=encryption[i]-temp;
			if(backString[i]==94)
				backString[i]=92;
		}
		char tempString[60]={0},endString[30]={0};
		sprintf(endString,"%d",end);
		for(unsigned int i=0;i<strlen(endString);i++)
			endString[i]+=50;
		sprintf(tempString,".%s.%c",endString,encryption[0]);
		strcat(backString,tempString);
		strcpy(getString,backString);
		return getString;
	}
	const char* decryptToken(const char* key,const char* token,char* buffer,unsigned int bufferLen)
	{
		char* temp=strchr((char*)token,'.');
		if(temp==NULL||key==NULL||token==NULL||buffer==NULL||bufferLen<strlen(token))
		{
			sprintf(err,"input wrong");
			return NULL;
		}
		if(backString!=NULL)
			free(backString);
		backString=(char*)malloc(sizeof(char)*strlen(token));
		memset(backString,0,sizeof(char)*strlen(token));
		char endString[20]={0};
		if(sscanf(temp+1,"%20[^.]",endString)<=0)
		{
			sprintf(err,"get time wrong");
			return NULL;
		}
		int keyTemp=0,end=0;
		for(unsigned int i=0;i<strlen(endString);i++)
			endString[i]-=50;
		sscanf(endString,"%d",&end);
		for(unsigned int i=0;i<strlen(key);i++)
			keyTemp+=key[i];
		end-=keyTemp;
		if(end-time(NULL)<=0)
		{
			sprintf(err,"over time");
			return NULL;
		}
		keyTemp=keyTemp%4;
		unsigned int i=0;
		for(i=0;i+token<temp;i++)
			if(token[i]!=92)
				backString[i]=token[i]+keyTemp;
			else
				backString[i]=97;
		backString[i+1]=0;
		if(backString[0]!=token[strlen(token)-1])
		{
			sprintf(err,"key wrong");
			return NULL;
		}
		strcpy(buffer,backString);
		return buffer;
	}
	inline const char* LastError()
	{
		return err;
	}
};
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
	inline const char* LastError()
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
	int getFileLen(const char* fileName)
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
	bool getFileMsg(const char* fileName,char* buffer,unsigned int bufferLen)
	{
		unsigned int i=0,len=0;
		len=this->getFileLen(fileName);
		FILE* fp=fopen(fileName,"rb");
		if(fp==NULL)
			return false;
		for(i=0;i<len&&i<bufferLen;i++)
			buffer[i]=fgetc(fp);
		buffer[i+1]=0;
		fclose(fp);
		return true;
	}
	bool fileStrstr(const char* fileName,const char* strFind)
	{
		int len=0;
		char* pstr=NULL;
		len=this->getFileLen(fileName);
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
		FILE* fp=fopen(fileName,"r");
		if(fp==NULL)
			return false;
		pbuffer=(char*)malloc(sizeof(char)*(len+10));
		char* ptemp=pbuffer;
		if(pbuffer==NULL)
			return false;
		memset(pbuffer,0,sizeof(char)*(len+5));
		if(false==this->getFileMsg(fileName,pbuffer,sizeof(char)*(len+10)))
			return false;
		while((*ptemp<65||*ptemp>122)&&ptemp<pbuffer+sizeof(char)*len)
			ptemp++;
		pstr=strstr(ptemp,strFind);
		if(pbuffer!=NULL)
		{
			free(pbuffer);
			pbuffer=NULL;
		}
		fclose(fp);
		if(pstr!=NULL)
			return true;
		else
			return false;
		return false;
	}
	static bool writeToFile(const char* fileName,const char* buffer,unsigned int writeLen)
	{
		FILE* fp=fopen(fileName,"wb+");
		if(fp==NULL)
			return false;
		for(unsigned int i=0;i<writeLen;i++)
			fputc(buffer[i],fp);
		fclose(fp);
		return true;
	}
};
class LogSystem{
public:
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
};
class HttpServer:private ServerTcpIp{
public:
	enum RouteType{
		ONEWAY,WILD,STATIC,
	};
	enum AskType{
		GET,POST,PUT,DELETETO,ALL,
	};
	struct RouteFuntion{
		AskType ask;
		RouteType type;
		char route[100];
		const char* path;
		void (*pfunc)(DealHttp&,HttpServer&,int num,void* sen,int&);
	};
private:
	RouteFuntion* array;
	RouteFuntion* pnowRoute;
	void* getText;
	unsigned int max;
	unsigned int now;
	int textLen;
	bool isDebug;
	bool isLongCon;
	bool isFork;
	void (*clientIn)(HttpServer&,int num,void* ip,int port);
	void (*clientOut)(HttpServer&,int num,void* ip,int port);
	void (*logFunc)(HttpServer&,const void*,int);
public:
	HttpServer(unsigned port,bool debug=false):ServerTcpIp(port)
	{
		getText=NULL;
		array=NULL;
		array=(RouteFuntion*)malloc(sizeof(RouteFuntion)*20);
		if(array==NULL)
			this->error="route wrong";
		else
			memset(array,0,sizeof(RouteFuntion)*20);
		now=0;
		max=20;
		isDebug=debug;
		isLongCon=true;
		isFork=false;
		textLen=0;
		clientIn=NULL;
		clientOut=NULL;
		logFunc=NULL;
		pnowRoute=NULL;
	}
	~HttpServer()
	{
		if(array!=NULL)
			free(array);
	}
	bool routeHandle(AskType ask,RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
	{
		if(strlen(route)>100)
			return false;
		if(max-now<=2)
		{
			array=(RouteFuntion*)realloc(array,sizeof(RouteFuntion)*(now+10));
			if(array==NULL)
				return false;
			max+=10;
		}
		array[now].type=type;
		array[now].ask=ask;
		strcpy(array[now].route,route);
		array[now].pfunc=pfunc;
		now++;
		return true;
	}
	bool get(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
	{
		if(strlen(route)>100)
			return false;
		if(max-now<=2)
		{
			array=(RouteFuntion*)realloc(array,sizeof(RouteFuntion)*(now+10));
			if(array==NULL)
				return false;
			max+=10;
		}
		array[now].type=type;
		array[now].ask=GET;
		strcpy(array[now].route,route);
		array[now].pfunc=pfunc;
		now++;
		return true;
	}
	bool post(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
	{
		if(strlen(route)>100)
			return false;
		if(max-now<=2)
		{
			array=(RouteFuntion*)realloc(array,sizeof(RouteFuntion)*(now+10));
			if(array==NULL)
				return false;
			max+=10;
		}
		array[now].type=type;
		array[now].ask=POST;
		strcpy(array[now].route,route);
		array[now].pfunc=pfunc;
		now++;
		return true;
	}
	bool all(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&))
	{
		if(strlen(route)>100)
			return false;
		if(max-now<=2)
		{
			array=(RouteFuntion*)realloc(array,sizeof(RouteFuntion)*(now+10));
			if(array==NULL)
				return false;
			max+=10;
		}
		array[now].type=type;
		array[now].ask=ALL;
		strcpy(array[now].route,route);
		array[now].pfunc=pfunc;
		now++;
		return true;
	}
	bool loadStatic(const char* route,const char* staticPath)
	{
		if(strlen(route)>100)
			return false;
		if(max-now<=2)
		{
			array=(RouteFuntion*)realloc(array,sizeof(RouteFuntion)*(now+10));
			if(array==NULL)
				return false;
			max+=10;
		}
		array[now].type=STATIC;
		array[now].ask=GET;
		strcpy(array[now].route,route);
		array[now].path=staticPath;
		array[now].pfunc=loadFile;
		now++;
		return true;
	}
	bool deletePath(const char* path)
	{
		if(strlen(path)>100)
			return false;
		if(max-now<=2)
		{
			array=(RouteFuntion*)realloc(array,sizeof(RouteFuntion)*(now+10));
			if(array==NULL)
				return false;
			max+=10;
		}
		array[now].type=STATIC;
		array[now].ask=GET;
		strcpy(array[now].route,path);
		array[now].pfunc=deleteFile;
		now++;
		return true;
	}
	int getCompleteMessage(const void* message,unsigned int messageLen,void* buffer,unsigned int buffLen,int sockCli)
	{
		if(message==NULL||buffer==NULL||buffLen<=0||message==0)
			return -1;
		unsigned int len=0;
		char* temp=NULL;
		if((temp=strstr((char*)message,"Content-Length"))==NULL)
			return -1;
		if(sscanf(temp+strlen("Content-Length")+1,"%d",&len)<=0)
			return -1;
		if((temp=strstr((char*)message,"\r\n\r\n"))==NULL)
			return -1;
		temp+=4;
		if(strlen(temp)>=len)
			return 0;
		if(strlen((char*)message)+len>buffLen)
			return -2;
		memcpy(buffer,message,messageLen);
		unsigned int leftLen=len-strlen(temp),getLen=0,all=0;
		while(leftLen>5||getLen<=0)
		{
			getLen=this->httpRecv(sockCli,(char*)buffer+messageLen+all,buffLen-messageLen-all);
			all+=getLen;
			leftLen-=getLen;
		}
		return len;
	}
	bool clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
	{
		if(clientIn!=NULL)
			return false;
		clientIn=pfunc;
		return true;
	}
	bool clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port))
	{
		if(clientOut!=NULL)
			return false;
		clientOut=pfunc;
		return true;
	}
	void run(unsigned int memory,unsigned int recBufLenChar,const char* defaultFile)
	{
		char* get=(char*)malloc(sizeof(char)*recBufLenChar);
		char* sen=(char*)malloc(sizeof(char)*memory*1024*1024);
		if(sen==NULL||get==NULL)
		{
			this->error="malloc get and sen wrong";
			return;
		}
		memset(get,0,sizeof(char)*recBufLenChar);
		memset(sen,0,sizeof(char)*memory*1024*1024);
		if(false==this->bondhost())
		{
			this->error="bound wrong";
			return;
		}
		if(false==this->setlisten())
		{
			this->error="set listen wrong";
			return;
		}
		this->getText=get;
		if(isDebug)
			printf("server is ok\n");
		if(isFork==false)
			while(1)
				this->epollHttp(get,recBufLenChar,memory,sen,defaultFile);
		else
			while(1)
				this->forkHttp(get,recBufLenChar,memory,sen,defaultFile);
		free(sen);
		free(get);
	}
	void changeSetting(bool debug,bool isLongCon,bool isForkModel)
	{
		this->isDebug=debug;
		this->isLongCon=isLongCon;
		this->isFork=isForkModel;
	}
	int httpSend(int num,void* buffer,int sendLen)
	{
		return this->sendSocket(num,buffer,sendLen);
	}
	int httpRecv(int num,void* buffer,int bufferLen)
	{
		return this->receiveSocket(num,buffer,bufferLen);
	}
	inline void* recText()
	{
		return this->getText;
	}
	inline int recLen()
	{
		return this->textLen;
	}
	inline const char* lastError()
	{
		return error;
	}
	inline bool disconnect(int soc)
	{
		return this->disconnectSocket(soc);
	}
private:
	inline RouteFuntion* getNowRoute()
	{
		return pnowRoute;
	}
	int func(int num,void* pget,void* sen,unsigned int senLen,const char* defaultFile,HttpServer& server)
	{
		static DealHttp http;
		AskType type=GET;
		int len=0,flag=2;
		char ask[200]={0};
		sscanf((char*)pget,"%100s",ask);
		if(strstr(ask,"GET")!=NULL)
		{
			http.getAskRoute(pget,"GET",ask,200);
			if(isDebug)
				printf("Get url:%s\n",ask);
			type=GET;
		}
		else if(strstr(ask,"POST")!=NULL)
		{
			http.getAskRoute(pget,"POST",ask,200);
			if(isDebug)
				printf("POST url:%s\n",ask);
			type=POST;
		}
		else if(strstr(ask,"PUT")!=NULL)
		{
			http.getAskRoute(pget,"PUT",ask,200);
			if(isDebug)
				printf("PUT url:%s\n",ask);
			type=PUT;
		}
		else if(strstr(ask,"DELETE")!=NULL)
		{
			http.getAskRoute(pget,"DELETE",ask,200);
			if(isDebug)
				printf("DELETE url:%s\n",ask);
			type=DELETETO;
		}
		void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&)=NULL;
		for(unsigned int i=0;i<now;i++)
		{
			if(array[i].type==ONEWAY&&(array[i].ask==type||array[i].ask==ALL))
			{
				if(strcmp(ask,array[i].route)==0)
				{
					pfunc=array[i].pfunc;
					pnowRoute=&array[i];
					break;
				}
			}
			else if(array[i].type==WILD&&(array[i].ask==type||array[i].ask==ALL))
			{
				if(strstr(ask,array[i].route)!=NULL)
				{
					pfunc=array[i].pfunc;
					pnowRoute=&array[i];
					break;
				}
			}
			else if(array[i].type==STATIC&&type==GET)
			{
				if(strstr(ask,array[i].route)!=NULL)
				{
					pfunc=array[i].pfunc;
					sprintf((char*)sen,"%s",array[i].path);
					pnowRoute=&array[i];
					break;
				}
			}
		}
		if(pfunc!=NULL)
		{
			if(pfunc!=loadFile)
				pfunc(http,*this,num,sen,len);
			else
				pfunc(http,*this,senLen,sen,len);
		}
		else
		{
			if(isDebug)
				printf("http:%s\n",http.analysisHttpAsk(pget));
			if(http.analysisHttpAsk(pget)!=NULL)
			{
				strcpy(ask,http.analysisHttpAsk(pget));
				flag=http.autoAnalysisGet((char*)pget,(char*)sen,senLen*1024*1024,defaultFile,&len);
			}
			if(flag==2)
			{
				if(isDebug)
				{
					LogSystem::recordFileError(ask);
					printf("404 get %s wrong\n",ask);
				}
			}
		}
		if(len==0)
			http.createSendMsg(DealHttp::NOFOUND,(char*)sen,senLen*1024*1024,NULL,&len);
		if(false==server.sendSocket(num,sen,len))
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
	void epollHttp(void* pget,int len,unsigned int senLen,void* pneed,const char* defaultFile)
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
		fd_set temp=fdClients;
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
							SOCKADDR_IN newaddr={0};
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(clientIn!=NULL)
								clientIn(*this,newClient,pget,newaddr.sin_port);
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						if(sRec>0)
						{
							this->textLen=sRec;
							func(fdClients.fd_array[i],pget,pneed,senLen,defaultFile,*this);
							if(isLongCon==false)
							{
						  		this->deleteFd(fdClients.fd_array[i]);
								FD_CLR(fdClients.fd_array[i],&fdClients);
								closesocket(fdClients.fd_array[i]);
							}
						}
						if(sRec<=0)
						{
							if(this->clientOut!=NULL)
							{
								int port=0;
								strcpy((char*)pget,this->getPeerIp(fdClients.fd_array[i],&port));
								clientOut(*this,fdClients.fd_array[i],pget,port);
							}
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);

						}
					}
				}
			}
		}
		else
			return ;
		return ;
	}
	void forkHttp(void* pget,int len,unsigned int senLen,void* pneed,const char* defaultFile)
	{//pthing is 0 out,1 in,2 say pnum is the num of soc,pget is rec,len is the max len of pget,pneed is others things
		fd_set temp=fdClients;
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
							SOCKADDR_IN newaddr={0};
							SOCKET newClient=accept(sock,(sockaddr*)&newaddr,&sizeAddr);
							FD_SET(newClient,&fdClients);
							this->addFd(newClient);
							strcpy((char*)pget,inet_ntoa(newaddr.sin_addr));
							if(clientIn!=NULL)
								clientIn(*this,newClient,pget,newaddr.sin_port);
						}
						else
							continue;
					}
					else
					{
						int sRec=recv(fdClients.fd_array[i],(char*)pget,len,0);
						if(sRec>0)
						{
							this->textLen=sRec;
							func(fdClients.fd_array[i],pget,pneed,senLen,defaultFile,*this);
							if(isLongCon==false)
							{
						  		this->deleteFd(fdClients.fd_array[i]);
								FD_CLR(fdClients.fd_array[i],&fdClients);
								closesocket(fdClients.fd_array[i]);
							}
						}
						if(sRec<=0)
						{
							if(this->clientOut!=NULL)
							{
								int port=0;
								strcpy((char*)pget,this->getPeerIp(fdClients.fd_array[i],&port));
								clientOut(*this,fdClients.fd_array[i],pget,port);
							}
							closesocket(fdClients.fd_array[i]);
							FD_CLR(fdClients.fd_array[i],&fdClients);
							this->deleteFd(fdClients.fd_array[i]);

						}
					}
				}
			}
		}
		else
			return ;
		return ;
	}
	static void loadFile(DealHttp& http,HttpServer& server,int senLen,void* sen,int& len)
	{
		char ask[200]={0},buf[200]={0},temp[200]={0};
		http.getAskRoute(server.recText(),"GET",ask,200);
		HttpServer::RouteFuntion& route=*server.getNowRoute();
		http.getWildUrl(ask,route.route,temp,200);
		sprintf(buf,"GET %s%s HTTP/1.1",route.path,temp);
		if(2==http.autoAnalysisGet(buf,(char*)sen,senLen*1024*1024,NULL,&len))
		{
			LogSystem::recordFileError(ask);
			printf("404 get %s wrong\n",buf);
		}
	}
	static void deleteFile(DealHttp& http,HttpServer&,int senLen,void* sen,int& len)
	{
		http.customizeAddTop(sen,senLen*1024*1024,DealHttp::STATUSFORBIDDEN,strlen("403 forbidden"),"text/plain");
		http.customizeAddBody(sen,senLen*1024*1024,"403 forbidden",strlen("403 forbidden"));
		len=strlen((char*)sen);
	}
};
}
#endif