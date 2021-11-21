#ifndef _HTTP_H_
#define _HTTP_H_
#include<string.h>
class DealHttp{
private:
	char ask[256];
	char* pfind;
public:
	enum FileKind{
		UNKNOWN=0,HTML=1,EXE=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP7=7,JSON=8,
	};
public:
	DealHttp();
	bool cutLineAsk(char* pask,const char* pcutIn);
	const char* analysisHttpAsk(void* pask,const char* pneed="GET",int needLen=3);
	inline char* findFirst(void* pask,const char* ptofind)
	{
		return strstr((char*)pask,ptofind);
	}
	char* findBackString(char* local,int len,char* word,int maxWordLen);
	void* customizeAddTop(void* buffer,int bufferLen,int statusNum,int contentLen,const char* contentType="application/json",const char* connection="keep-alive");
	void* customizeAddHead(void* buffer,int bufferLen,const char* key,const char* value);
	int customizeAddBody(void* buffer,int bufferLen,const char* body,unsigned int bodyLen);
	bool setCookie(void* buffer,int bufferLen,const char* key,const char* value,int liveTime=-1,const char* path=NULL,const char* domain=NULL);
	void createTop(FileKind kind,char* ptop,int* topLen,int fileLen);
	bool createSendMsg(FileKind kind,char* buffer,const char* pfile,int* plong);
	char* findFileMsg(const char* pname,int* plen,char* buffer);
	int getFileLen(const char* pname);
	int autoAnalysisGet(const char* message,char* psend,const char* pfirstFile,int* plen);
	const char* getKeyValue(const void* message,const char* key,char* value,int maxValueLen,bool onlyFromBody=false);
	const char* getKeyLine(const void* message,const char* key,char* line,int maxLineLen,bool onlyFromBody=false);
	const char* getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen);
	const char* getRouteValue(const void* routeMeg,const char* key,char* value,unsigned int valueLen);
	const char* getWildUrl(const void* getText,const char* route,char* buffer,int maxLen);
	int getRecFile(const void* message,char* fileName,int nameLen,char* buffer,int bufferLen);
	static void dealUrl(const char* url,char* urlTop,char* urlEnd);
	static const char* urlDecode(char* srcString);
};
class LogSystem{
public:
	struct CliLog{
		int socketCli;
		int time;
		char ip[20];
	};
public:
	static bool dealAttack(int isUpdate,int socketCli,int maxTime);//check if accket
	static bool attackLog(int port,const char* ip,const char* pfileName);//log accket
	static bool recordFileError(const char* filename);
};
class Json{
private:
	char* buffer;
	char word[30];
	const char* text;
	const char* obj;
	const char* error;
	unsigned int nowLen;
	unsigned int maxLen;
public:
	enum TypeJson{
		INT=0,FLOAT=1,ARRAY=2,OBJ=3,STRING=4,STRUCT=5,
	};
	struct Object{
		TypeJson type;
		TypeJson arrTyp;
		const char* key;
		int valInt;
		float valFlo;
		unsigned int floOut;
		unsigned int arrLen;
		const char* valStr;
		void** array;
		Object* pobj;
	public:
		Object()
		{
			type=INT;
			arrTyp=INT;
			key=NULL;
			valFlo=0;
			valInt=0;
			valStr=NULL;
			array=NULL;
			pobj=NULL;
		}
	};
public:
	Json();
	Json(const char* jsonText);
	~Json();
	bool init(unsigned int bufferLen);
	void addOBject(const Object& obj);
	bool addKeyValue(const char* key,const char* value);
	bool addKeyValInt(const char* key,int value);
	bool addKeyObj(const char* key,const char* value);
	bool addKeyValFloat(const char* key,float value,int output);
	void createObject(char* pbuffer,int bufferLen,const Object& obj);
	int createObjInt(char* pbuffer,unsigned int bufferLen,const char* key,int value);
	int createObjFloat(char* pbuffer,unsigned int bufferLen,const char* key,float value,int output=1);
	int createObjValue(char* pbuffer,unsigned int bufferLen,const char* key,const char* value);
	bool createObjArray(char* pbuffer,unsigned int bufferLen,TypeJson type,const char* key,void** array,unsigned int arrLen,unsigned int floatNum=1);
	int createObjObj(char* pbuffer,unsigned int bufferLen,const char* key,const char* value);
	bool addArray(TypeJson type,const char* key,void** array,unsigned int arrLen,unsigned int floatNum=1);
	inline const char* resultText()
	{
		return buffer;
	}
	inline const char* getLastError()
	{
		return this->error;
	}
	bool jsonToFile(const char* fileName);
	const char* operator[](const char* key);
	float getValueFloat(const char* key,bool& flag);
	int getValueInt(const char* key,bool& flag);
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
#endif
