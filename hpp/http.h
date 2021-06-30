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
	char* pfile;
public:
	DealHttp();
	~DealHttp();
	bool cutLineAsk(char* pask,const char* pcutIn);
	const char* analysisHttpAsk(void* pask,const char* pneed="GET",int needLen=3);
	inline char* findFirst(void* pask,const char* ptofind)
	{
		return strstr((char*)pask,ptofind);
	}
    int getFileLen(const char* pname);
	char* findBackString(char* ps,int len,char* word);
	void createTop(int kind,char* ptop,int* topLen,int fileLen);
	bool createSendMsg(int kind,char* pask,const char* pfile,int* plong);
	char* findFileMsg(const char* pname,int* plen);
	int autoAnalysisGet(const char* pask,char* psend,const char* pfirstFile,int* plen);
};
class DealAttack{
public:
	static bool dealAttack(int isUpdate,int socketCli,int maxTime);//check if accket
	static bool attackLog(int port,const char* ip,const char* pfileName);//log accket
};
#endif
