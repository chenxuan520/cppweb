#ifndef _HTTP_H_
#define _HTTP_H_
#include<string.h>
#include<string>
#include<stack>
#include<vector>
#include<unordered_map>
namespace cppweb{
class DealHttp{
private:
	char ask[256];
	char* pfind;
	const char* error;
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
public:
	DealHttp();
	bool cutLineAsk(char* message,const char* pcutIn);
	const char* analysisHttpAsk(void* message,const char* pneed="GET");
	inline char* findFirst(void* message,const char* ptofind)
	{
		return strstr((char*)message,ptofind);
	}
	char* findBackString(char* local,int len,char* word,int maxWordLen);
	void* customizeAddTop(void* buffer,unsigned int bufferLen,int statusNum,unsigned int contentLen,const char* contentType="application/json",const char* connection="keep-alive",const char* staEng=NULL);
	void* customizeAddHead(void* buffer,unsigned int bufferLen,const char* key,const char* value);
	int customizeAddBody(void* buffer,unsigned int bufferLen,const char* body,unsigned int bodyLen);
	bool setCookie(void* buffer,unsigned int bufferLen,const char* key,const char* value,int liveTime=-1,const char* path=NULL,const char* domain=NULL);
	const char* getCookie(void* recText,const char* key,char* value,unsigned int valueLen);
	void createTop(FileKind kind,char* ptop,unsigned int bufLen,int* topLen,unsigned int fileLen);
	bool createSendMsg(FileKind kind,char* buffer,unsigned int bufferLen,const char* pfile,int* plong);
		int createDatagram(const Datagram& gram,void* buffer,unsigned bufferLen);
	char* findFileMsg(const char* pname,int* plen,char* buffer,unsigned int bufferLen);
	int getFileLen(const char* pname);
	void getRequestMsg(void* message,Request& request);
	int autoAnalysisGet(const char* message,char* psend,unsigned int bufferLen,const char* pfirstFile,int* plen);
	const char* getKeyValue(const void* message,const char* key,char* value,unsigned int maxValueLen,bool onlyFromBody=false);
	const char* getKeyLine(const void* message,const char* key,char* line,unsigned int maxLineLen,bool onlyFromBody=false);
	const char* getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen);
	const char* getRouteValue(const void* routeMeg,const char* key,char* value,unsigned int valueLen);
	const char* getWildUrl(const void* getText,const char* route,char* buffer,unsigned int maxLen);
	int getRecFile(const void* message,char* fileName,int nameLen,char* buffer,unsigned int bufferLen);
	static void dealUrl(const char* url,char* urlTop,char* urlEnd,unsigned int topLen,unsigned int endLen);
	static const char* urlDecode(char* srcString);
};
class LogSystem{
public:
	static bool recordFileError(const char* filename);
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
	Json();
	Json(const char* jsonText);
	~Json();
	const char* formatPrint(const Object* exmaple,unsigned buffLen);
	Object* operator[](const char* key);
	bool addKeyVal(char* obj,TypeJson type,const char* key,...);
	char* createObject(unsigned maxBuffLen);
	char* createArray(unsigned maxBuffLen,TypeJson type,unsigned arrLen,void* arr);
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
	Object* analyseObj(char* begin,char* end);
	TypeJson analyseArray(char* begin,char* end,std::vector<Object*>& array);
	void findString(const char* begin,char* buffer,unsigned buffLen);
	void findNum(const char* begin,TypeJson type,void* pnum);
	inline TypeJson judgeNum(const char* begin,const char* end)
	{
		for(unsigned i=0;i+begin<end;i++)
			if(begin[i]=='.')
				return FLOAT;
		return INT;
	}
	void deleteSpace();
	void deleteNode(Object* root);
	void deleteComment();
	bool pairBracket();
	bool printObj(char* buffer,const Object* obj);
	bool printArr(char* buffer,TypeJson type,const std::vector<Object*>& arr);
};
class WebToken{
private:
	char* backString;
	char err[30];
public:
	WebToken();
	const char* createToken(const char* key,const char* encryption,char* getString,unsigned int stringLen,unsigned int liveSecond);
	const char* decryptToken(const char* key,const char* token,char* buffer,unsigned int bufferLen);
	inline const char* LastError()
	{
		return err;
	}
};
class FileGet{
private:
	char* pbuffer;
public:
	FileGet();
	~FileGet();
	int getFileLen(const char* fileName);
	bool getFileMsg(const char* fileName,char* buffer,unsigned int bufferLen);
	bool fileStrstr(const char* fileName,const char* strFind);
	static bool writeToFile(const char* fileName,const char* buffer,unsigned int writeLen);
};

}
#endif
