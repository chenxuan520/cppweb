#ifndef _HTTP_H_
#define _HTTP_H_
#include<string.h>
struct CliMsg{ 
	char hao[20];
	char mi[20];
	int flag;
	char chat[100];
};
struct CliLog{
	int socketCli;
	int time;
	char ip[20];
};
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
	void createTop(FileKind kind,char* ptop,int* topLen,int fileLen);
	bool createSendMsg(FileKind kind,char* pask,const char* pfile,int* plong);
	char* findFileMsg(const char* pname,int* plen,char* buffer);
	int getFileLen(const char* pname);
	int autoAnalysisGet(const char* pask,char* psend,const char* pfirstFile,int* plen);
	const char* getKeyValue(const void* message,const char* key,char* value,int maxValueLen);
	const char* getKeyLine(const void* message,const char* key,char* line,int maxLineLen);
	const char* getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen);
	const char* getRouteValue(const void* routeMeg,const char* key,char* value,unsigned int valueLen);
	const char* getWildUrl(const void* getText,const char* route,char* buffer,int maxLen);
	static void dealUrl(const char* url,char* urlTop,char* urlEnd);
};
class DealAttack{
public:
	static bool dealAttack(int isUpdate,int socketCli,int maxTime);//check if accket
	static bool attackLog(int port,const char* ip,const char* pfileName);//log accket
};
class Json{
private:
	char* buffer;
	char word[30];
	const char* text;
	unsigned int nowLen;
	unsigned int maxLen;
public:
	Json();
	Json(const char* jsonText);
	~Json();
	bool jsonInit(unsigned int bufferLen);
	bool addKeyValue(const char* key,const char* value);
	bool addKeyValInt(const char* key,int value);
	bool addKeyValFloat(const char* key,float value,int output);
	const char* endJson();
	bool jsonToFile(const char* fileName);
	const char* operator[](const char* key);
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
#endif
