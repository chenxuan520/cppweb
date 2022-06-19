#include "../../hpp/cppweb.h"
#include "./config.h"
#include <dirent.h>
#include <ctime>
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
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	char path[128]={0};
	http.getWildUrl(server.recText(),server.getNowRoute()->route,path,sizeof(char)*128);
	req.askPath=".";
	req.askPath+=path;
	cout<<"path:"<<req.askPath<<endl;
	if(req.askPath.find("web-link")!=req.askPath.npos)
		req.askPath="./link";
	dirent* ptr=NULL;
	DIR* dir=NULL;
	struct stat temp;
	auto now=stat(req.askPath.c_str(),&temp);
	if(now<0)
	{
		Json json={{"status","stat wrong"}};
		http.gram.json(DealHttp::STATUSOK,json());
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
		json[ "list" ]=file;
		printf("json:%s\n",json());
		http.gram.json(DealHttp::STATUSOK,json());
	}
	else
	{
		Json json={{"status","unknown file"}};
		http.gram.json(DealHttp::STATUSOK,json());
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
		http.gram.html(DealHttp::STATUSOK,FileGet::getFileString("../index.html"));
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
		http.gram.html(DealHttp::STATUSOK,FileGet::getFileString("../index.html"));
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
		http.gram.json(DealHttp::STATUSOK,json());
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
	http.gram.html(DealHttp::STATUSOK,FileGet::getFileString("../jump.html"));
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
		http.gram.json(DealHttp::STATUSOK,json());
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
		http.gram.statusCode=DealHttp::STATUSOK;
		return;
	}
	if(!isDelete)
		flag=mkdir(req.askPath.c_str(),S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	else if(isDelete)
		flag=remove(req.askPath.c_str());
	else
	{
		Json json={{"status",(const char*)strerror(errno)}};
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	if(flag==0)
	{
		Json json={{"status","ok"}};
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	else
	{
		Json json={{"status",(const char*)strerror(errno)}};
		http.gram.json(DealHttp::STATUSOK,json());
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
		http.gram.json(DealHttp::STATUSOK,json());
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
	auto root=json.getRootObj();
	if(root["newpath"]!=Json::npos)
		newStr=root["newpath"].strVal;
	else
	{
		json[ "status" ]="wrong";
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	if(newStr.find("..")!=newStr.npos||req.askPath.find("..")!=req.askPath.npos)
		json[ "status" ]="wrong path";
	else
	{
		newStr.insert(newStr.begin(),1,'.');
		auto flag=rename(req.askPath.c_str(),newStr.c_str());
		if(flag!=0)
			json[ "status" ]=(const char*)strerror(errno);
		else
			json[ "status" ]="ok";
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
	http.req.analysisRequest(server.recText());
	auto value=http.req.formValue("passwd");
	if(value.size()==0)
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
		http.gram.cookie["disk"]=http.designCookie(_config.passwd.c_str(),_config.tokenTime);
		http.gram.html(DealHttp::STATUSOK,FileGet::getFileString("../index.html"));
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
		json[ "status" ]=req.error;
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	json.analyseText(req.body);
	auto root=json.getRootObj();
	if(json.lastError()!=NULL||root["content"]==Json::npos)
	{
		json[ "status" ]=json.lastError();
		http.gram.json(DealHttp::STATUSOK,json());
	}
	std::string& content=root["content"].strVal;
	FileGet::writeToFile("../text.txt",content.c_str(),content.size());
	json[ "status" ]="ok";
	http.gram.json(DealHttp::STATUSOK,json());
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-06-03 16:44:07
* description:create web link for res
***********************************************/
void webLink(HttpServer& server,DealHttp& http,int soc)
{
	auto flag=http.req.analysisRequest(server.recText(),true);
	auto pos=http.req.askPath.find("web-link/");
	int len=0;
	if(!flag||pos==http.req.askPath.npos)
	{
		http.gram.json(DealHttp::STATUSNOFOUND,Json::createJson(Json::Node()={{"status","message wrong"}}));
		return;
	}
	auto temp=http.req.askPath.substr(pos+strlen("web-link/"));
	string file="./link/"+temp;
	http.createSendMsg(DealHttp::UNKNOWN,(char*)server.getSenBuffer(),server.getMaxSenLen(),file.c_str(),&len);
	if(len==0)
	{
		http.gram.noFound();
		len=http.createDatagram(http.gram,server.getSenBuffer(),server.getMaxSenLen());
	}
	if(len>0)
		server.httpSend(soc,server.getSenBuffer(),len);
}
/***********************************************
* description:create soft link
***********************************************/
string createLink(string from,string link)
{
	auto flag=symlink(from.c_str(),link.c_str());
	cout<<"from:"<<from<<" link:"<<link;
	if(flag!=0)
		return "message wrong";
	else
		return string("/web-link/")+link;
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-06-03 17:15:57
* description:route to create link
***********************************************/
void apiLink(HttpServer& server,DealHttp& http,int)
{
	char path[128]={0};
	auto flag=http.req.analysisRequest(server.recText(),true);
	if(!flag)
	{
		http.gram.json(DealHttp::STATUSNOFOUND,Json::createJson(Json::Node()={{"status","message wrong"}}));
		return;
	}
	http.getWildUrl(server.recText(),server.getNowRoute()->route,path,sizeof(char)*128);
	char pwd[256]={0};
	flag=getcwd(pwd,256);
	if(!flag)
	{
		http.gram.json(DealHttp::STATUSNOFOUND,Json::createJson(Json::Node()={{"status","message wrong"}}));
		return;
	}
	http.req.askPath=pwd;
	http.req.askPath+=path;
	auto pos=http.req.askPath.find(".");
	if(pos==http.req.askPath.npos)
	{
		http.gram.json(DealHttp::STATUSOK,Json::createJson({{"status","file wrong"}}));
		return;
	}
	auto temp=http.req.askPath.substr(pos);
	auto now=to_string(time(NULL))+temp;
	auto link=string("./link/")+now;
	createLink(http.req.askPath,link);
	Json json={{"status",string("/web-link/"+now)}};
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
		json[ "status" ]=req.error;
		json[ "content" ]=nullptr;
		http.gram.json(DealHttp::STATUSOK,json());
		return;
	}
	json[ "status" ]="ok";
	FileGet file;
	/* printf("con:%s\n",file.getFileString("../text.txt").c_str()); */
	json[ "content" ]=file.getFileString("../text.txt");
	printf("result:%s\n",json());
	http.gram.json(DealHttp::STATUSOK,json());
	return;
}
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-04-27 18:44:00
* description:log out from the disk
***********************************************/
void loginOut(HttpServer&,DealHttp& http,int)
{
	http.gram.cookie["disk"]=http.designCookie("",0);
	http.gram.html(DealHttp::STATUSOK,FileGet::getFileString("../login.html"));
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
	else if(req.method=="GET"&&req.askPath.find("web-link/")!=req.askPath.npos)
		webLink(server,http,soc);
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
/***********************************************
* Author: chenxuan-1607772321@qq.com
* change time:2022-06-19 22:03:50
* description:rewrite the main function
***********************************************/
int _main(ArgcDeal& arg){
	if(arg.getOption("d")){
		ProcessCtrl::backGround();
	}
	if(arg.getOption("g")){
		ProcessCtrl::guard();
	}
	if(false==configServer("./config.json"))
	{
		printf("find config.json wrong\n");
		return 0;
	}
	HttpServer server(_config.port,true);//input the port bound
	if(_config.isLog)
		server.setLog(LogSystem::recordRequest,LogSystem::recordRequest);
	if(_config.isVerify)
		server.setMiddleware(middleware);
	server.post("/message*",nowPwdFile);
	server.post("/upload*",upload);
	server.post("/mkdir*",mkdirNow);
	server.post("/delete*",mkdirNow);
	server.post("/move*",moveFile);
	server.post("/login",loginIn);
	server.post("/logout",loginOut);
	server.post("/link*",apiLink);
	server.get("/*",sendHtml);
	auto group=server.createGroup("/edit");
	{
		group.post("/save",saveEdit);
		group.post("/get",getEdit);
	}
	server.run();
	if(server.lastError()!=NULL)
	{
		std::cout<<server.lastError()<<std::endl;
		return -1;
	}
    return 0;
}
