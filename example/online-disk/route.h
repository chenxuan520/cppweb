#include "../../hpp/cppweb.h"
#include "./config.h"
#include <dirent.h>
#include <sys/stat.h>
using namespace cppweb;
using namespace std;
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-24 09:07:41
* description:sent the now file route
***********************************************/
void nowPwdFile(HttpServer& server,DealHttp& http,int)
{
	DealHttp::Request req;
	if(false==http.analysisRequest(req,server.recText()))
	{
		Json json={{"status","analysisHttpAsk wrong"}};
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
	char path[128]={0};
	http.getWildUrl(server.recText(),server.getNowRoute()->route,path,sizeof(char)*128);
	req.askPath=".";
	req.askPath+=path;
	cout<<"path:"<<req.askPath<<endl;
	dirent* ptr=NULL;
	DIR* dir=NULL;
	struct stat temp;
	auto now=stat(req.askPath.c_str(),&temp);
	if(now<0)
	{
		Json json={{"status","stat wrong"}};
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
	else if(S_ISREG(temp.st_mode))
	{
		FileGet file;
		int len=file.getFileLen(req.askPath.c_str());
		http.gram.body=string(file.getFileBuff(req.askPath.c_str()),len);
		http.gram.fileLen=len;
		http.gram.typeFile=DealHttp::UNKNOWN;
		return;
	}
	else if(S_ISDIR(temp.st_mode))
	{
		Json json={{"status","ok"}};
		dir=opendir(req.askPath.c_str());
		vector<string> file;
		while((ptr=readdir(dir))!=NULL)
		{
			//jump if file begin with '.'
			if(ptr->d_name[0] == '.')
				continue;
			file.push_back((char*)ptr->d_name);
		}
		closedir(dir);
		json("list")=file;
		printf("json:%s\n",json());
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
	}
	else
	{
		Json json={{"status","unknown file"}};
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-24 17:27:06
* description: sen the get text
***********************************************/
void sendHtml(HttpServer& server,DealHttp& http,int)
{
	DealHttp::Request req;
	if(false==http.analysisRequest(req,server.recText()))
	{
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
		return;
	}
	if(req.askPath=="/edit")
	{
		http.gram.html(DealHttp::STATUSOK,FileGet::getFileString("../edit.html"));
		return;
	}
	struct stat temp;
	cout<<"get:"<<req.askPath<<endl;
	req.askPath="."+req.askPath;
	auto now=stat(req.askPath.c_str(),&temp);
	if(now<0)
	{
		FileGet file;
		http.gram.body=file.getFileBuff("../index.html");
		http.gram.typeFile=DealHttp::HTML;
		return;
	}
	else if(S_ISREG(temp.st_mode))
	{
		FileGet file;
		int len=file.getFileLen(req.askPath.c_str());
		http.gram.body=string(file.getFileBuff(req.askPath.c_str()),len);
		http.gram.fileLen=len;
		http.gram.typeFile=DealHttp::UNKNOWN;
		return;
	}
	else
	{
		FileGet file;
		int len=file.getFileLen("../index.html");
		http.gram.body=string(file.getFileBuff("../index.html"),len);
		http.gram.fileLen=len;
		http.gram.typeFile=DealHttp::HTML;
		return;
	}
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-24 18:58:36
* description: upload file to server
***********************************************/
void upload(HttpServer& server,DealHttp& http,int soc)
{
    char name[100]={0};
	DealHttp::Request req;
	if(false==http.analysisRequest(req,server.recText()))
	{
		Json json={{"status","analysisHttpAsk wrong"}};
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
	if(req.head.find("Content-Length")==req.head.end())
	{
		http.gram.body="message wrong";
		return;
	}
	char path[128]={0};
	http.getWildUrl(server.recText(),server.getNowRoute()->route,path,sizeof(char)*128);
	req.askPath=".";
	req.askPath+=path;
	int flen=0;
	sscanf(req.head["Content-Length"].c_str(),"%d",&flen);
	void *temp=malloc(sizeof(char)*(flen+1000));
	if(temp==NULL)
	{
		http.gram.statusCode=DealHttp::STATUSNOFOUND;
		return;
	}
	memset(temp,0,sizeof(char)*(flen+1000));
	unsigned len=server.getCompleteMessage(soc);
    flen=http.getRecFile(server.recText(),len,name,100,(char*)temp,sizeof(char)*(flen+1000));
	req.askPath+=name;
	cout<<"file:"<<req.askPath<<endl;
    FileGet::writeToFile(req.askPath.c_str(),(char*)temp,flen);
	http.gram.typeFile=DealHttp::HTML;
	FileGet file;
	http.gram.body=file.getFileBuff("../jump.html");
	http.gram.typeFile=DealHttp::HTML;
	free(temp);
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-24 19:45:15
* description:mkdir to server
***********************************************/
void mkdirNow(HttpServer& server,DealHttp& http,int)
{
	DealHttp::Request req;
	if(false==http.analysisRequest(req,server.recText()))
	{
		Json json={{"status","analysisHttpAsk wrong"}};
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
	bool isDelete=false;
	isDelete=strstr(req.askPath.c_str(),"delete")!=NULL;
	char path[128]={0};
	http.getWildUrl(server.recText(),server.getNowRoute()->route,path,sizeof(char)*128);
	req.askPath=".";
	req.askPath+=path;
	int flag=0;
	if(strstr(req.askPath.c_str(),"..")!=NULL)
	{
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body="{\"status\":\"delete wrong\"}";
		return;
	}
	if(!isDelete)
		flag=mkdir(req.askPath.c_str(),S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	else if(isDelete)
		flag=remove(req.askPath.c_str());
	else
	{
		Json json={{"status",(const char*)strerror(errno)}};
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
	if(flag==0)
	{
		Json json={{"status","ok"}};
		http.gram.statusCode=DealHttp::STATUSOK;
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
	else
	{
		Json json={{"status",(const char*)strerror(errno)}};
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-04-02 21:00:23
* description:deal move file or dir ask
***********************************************/
void moveFile(HttpServer& server,DealHttp& http,int)
{
	DealHttp::Request req;
	if(false==http.analysisRequest(req,server.recText()))
	{
		Json json={{"status","analysisHttpAsk wrong"}};
		http.gram.typeFile=DealHttp::JSON;
		http.gram.body=json();
		return;
	}
	bool isDelete=false;
	isDelete=strstr(req.askPath.c_str(),"delete")!=NULL;
	char path[128]={0};
	http.getWildUrl(server.recText(),server.getNowRoute()->route,path,sizeof(char)*128);
	req.askPath=".";
	req.askPath+=path;
	Json json(req.body);
	string newStr;
	if(json["newpath"]!=NULL)
		newStr=json["newpath"]->strVal;
	else
	{
		json("status")="wrong";
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	if(newStr.find("..")!=newStr.npos||req.askPath.find("..")!=req.askPath.npos)
		json("status")="wrong path";
	else
	{
		newStr.insert(newStr.begin(),1,'.');
		auto flag=rename(req.askPath.c_str(),newStr.c_str());
		if(flag!=0)
			json("status")=(const char*)strerror(errno);
		else
			json("status")="ok";
	}
	http.gram.json(DealHttp::STATUSOK,json());
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-25 12:03:24
* description:login in 
***********************************************/
void loginIn(HttpServer& server,DealHttp& http,int)
{
	char value[128]={0};
	if(NULL==http.getKeyValue(server.recText(),"passwd",value,128))
	{
		http.gram.body="Verify identity wrong";
		return;
	}
	if(_config.passwd!=value)
	{
		http.gram.body="Verify passwd wrong";
		return;
	}
	else
	{
		http.gram.statusCode=DealHttp::STATUSOK;
		http.gram.typeFile=DealHttp::HTML;
		http.gram.cookie["disk"]=http.designCookie(_config.passwd.c_str(),_config.tokenTime);
		FileGet file;
		http.gram.body=file.getFileBuff("../index.html");
		if(http.gram.body.size()==0)
			printf("index wrong\n");
		return;
	}
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-04-06 20:36:18
* description:post method to change edit
***********************************************/
void saveEdit(HttpServer& server,DealHttp& http,int)
{
	DealHttp::Request req;
	Json json;
	if(false==req.analysisRequest(server.recText()))
	{
		json("status")=req.error;
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	json.analyseText(req.body);
	if(json.lastError()!=NULL||json["content"]==NULL)
	{
		json("status")=json.lastError();
		http.gram.json(DealHttp::STATUSOK,json());
	}
	std::string& content=json["content"]->strVal;
	FileGet::writeToFile("../text.txt",content.c_str(),content.size());
	json("status")="ok";
	http.gram.json(DealHttp::STATUSOK,json());
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-04-06 21:12:39
* description:get file message
***********************************************/
void getEdit(HttpServer& server,DealHttp& http,int)
{
	DealHttp::Request req;
	Json json;
	if(false==req.analysisRequest(server.recText()))
	{
		json("status")=req.error;
		json("content")=nullptr;
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	json("status")="ok";
	FileGet file;
	/* printf("con:%s\n",file.getFileString("../text.txt").c_str()); */
	json("content")=file.getFileString("../text.txt");
	printf("result:%s\n",json());
	http.gram.json(DealHttp::STATUSOK,json());
	return;
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-03-25 12:54:14
* description:middware get cookie 
***********************************************/
void middleware(HttpServer& server,DealHttp& http,int soc)
{
	static DealHttp::Request req;
	string getpwd=http.getCookie(server.recText(),"disk");
	http.analysisRequest(req,server.recText(),true);
	if(req.method=="POST"&&req.askPath.find("login")!=req.askPath.npos)
		server.continueNext(soc);
	else if(getpwd!=_config.passwd)
	{
		int len=0;
		http.createSendMsg(DealHttp::HTML,(char*)server.getSenBuffer(),server.getMaxSenLen(),"../login.html",&len);
		server.httpSend(soc,server.getSenBuffer(),len);
		return;
	}
	else
		server.continueNext(soc);
}
