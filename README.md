# cppweb框架介绍

## 作者

**chenxuan**

![dog](https://i.loli.net/2021/10/25/7pQUDsB12GE4tgx.png)

## 创作初衷

自己在搭建C++服务器的时候发现，boost库太庞大对新手不够友好，libevent应用于专业化对新人不友好，用c++开发http后端十分繁琐而且代码复用率低，在使用go的gin框架的时候 ，就萌发了自己写一个类似后端轻量库的想法 

## 官方网站

[http://cpp.androidftp.top](http://cpp.androidftp.top) 

## 框架特点

- 具有Linux和windows跨平台特点
- 轻量级 ，只有几个静态库，代码不到5000行,可直接使用头文件操作
- 扩展性好，自由订制处理http响应 
- 安装方便,核心只有hpp/cppweb.h一个头文件
- 学习成本低，半个小时就可掌握用法 
- 文档详细中文 ，代码全部开源 

## 项目结构

- hpp中的cppweb.h为所有代码,包含该头文件就可以使用
- old文件夹为1.0版本的头文件和源文件,不推荐
- example为使用实例,推荐优先阅读
- doc为说明文档，包含函数和类的用法
- test文档为测试文档
- class是为编译的一些类，测试中
- bin是可执行文件  
- install.sh为服务器运行脚本
- windows文件夹是windows版本

## 项目说明

- 该项目可用于个人搭建小网站后端服务
- 本人只是一名大二的学生，框架的不足之处恳请大家通过issue发给我一定认真改进
- 这个和图形库类似只是给初学者简便的创作
- 代码不足之处请务必联系我改进
- 在使用该项目之前请阅读doc下的文档

## 项目特点

1. 使用C++编写,拥有较快的运行速度

2. 支持使用自定义中间件

3. 支持cookie的生成和读取

4. 包含一个[json的解析生成格式化库](https://gitee.com/chenxuan520/cppjson)

5. 支持通过路由管理请求

6. 支持日志生成和实现,日志系统约为30万条每秒

7. 具有io复用,多进程,线程池三种模式

8. 包含线程池和线程池服务器

9. 支持https连接(客户端和服务端都可)

## 搭建服务器

- ./install.sh为安装脚本,运行就可以安装

## 框架介绍

#### 搭建服务器

```cpp
 #include "../../hpp/cppweb.h"//包含头文件
using namespace cppweb;
int main()  
{  
    HttpServer server(5200,true);//输入运行端口以及是否开启打印的调试模式
    server.run("index.html");//输入访问路径为 / 时默认文件
    return 0; //没有错会一直运行,除非出错,可以用lastError获取错取
}  
```

#### 搭建https

```cpp
#define CPPWEB_OPENSSL
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
    HttpServer server(5201,true,HttpServer::THREAD,5);
    server.loadKeyCert("./cacert.pem","./privkey.pem","123456");
    //第一个是证书位置,第二个是私钥位置,第三个是密码(没有可以不填)
    server.run("./index.html");
    return 0;
}
```

- 需要安装openssl

- 需要包含头文件之前定义宏

#### 路由设置

```cpp
    server.get("/lam*",[](HttpServer&,DealHttp& http,int)->void{
        http.gram.body="text";//设置报文内容
    });
```

- 支持lam表达式和普通函数

- 支持路由的*扩展

#### 中间件设置

```cpp
void pfunc(HttpServer& server,DealHttp& http,int soc)
{
    http.head["try"]="en";
    server.continueNext(soc);//继续执行后面默认操作,也可以不执行
}
int main()
{
    HttpServer server(5200,true);
    server.setMiddleware(pfunc);//设置中间件
    server.get("/root",[](HttpServer&,DealHttp& http,int){
               http.gram.body="text";
               });
    server.run("./index.html");
    return 0;
}
```

- 支持中间件对报文统一处理

#### 日志设置

```cpp
    server.setLog(LogSystem::recordRequest,NULL);
    server.get("/stop",[](HttpServer& server,DealHttp&,int){
               server.stopServer();
               });
```

- LogSystem有一些模板,也可以自己设置

#### 解析报文

```cpp
void func(HttpServer& server,DealHttp& http,int)
{
    DealHttp::Request req;
    http.analysisRequest(req,server.recText());
    printf("old:%s\n",(char*)server.recText());
    printf("new:%s %s %s\n",req.method.c_str(),req.askPath.c_str(),req.version.c_str());
    for(auto iter=req.head.begin();iter!=req.head.end();iter++)
        printf("%s:%s\n",iter->first.c_str(),iter->second.c_str());
    printf("body:%s\n",req.body);
    http.gram.statusCode=DealHttp::STATUSOK;
    http.gram.typeFile=DealHttp::JSON;
    http.gram.body="{\"ha\":\"ha\"}";
}
int main()  
{  
    HttpServer server(5200,true);//input the port bound
    server.all("/root",func);
    server.run("./index.html");
    if(server.lastError()!=NULL)
    {
        std::cout<<server.lastError()<<std::endl;
        return -1;
    }
    return 0; 
}  
```

- 通过http内置的结构体来解析

#### 设置cookie

```cpp
void cookie(HttpServer& server,DealHttp& http,int)
{
    char buffer[100]={0};
    http.getCookie(server.recText(),"key",buffer,100);
    if(strlen(buffer)==0)
    {
        http.gram.body="ready to setting cookie";
        http.gram.cookie["key"]=http.designCookie("cookie ok",10);
        return;
    }
    Json json={
        {"key",(const char*)buffer},
        {"status","ok"}
    };
    http.gram.body=json();
    http.gram.typeFile=DealHttp::JSON;
}
int main()  
{  
    HttpServer server(5200,true);
    server.get("/cookie",cookie);
    server.run("index.html");
    return 0; 
}  
```

#### 客户端创建

```cpp
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../../hpp/cppweb.h"
using namespace std;
using namespace cppweb;
int main()
{
    char ip[30]={0},topUrl[100]={0},endUrl[100]={0};
    string str="http://chenxuanweb.top/";
    DealHttp::dealUrl(str.c_str(),topUrl,endUrl,100,100);
    ClientTcpIp::getDnsIp(topUrl,ip,30);
    unsigned port=5200;
    if(strstr(str.c_str(),"https")!=NULL)
        port=443;
    ClientTcpIp client(ip,port);
    cout<<client.lastError();
    DealHttp http;
    DealHttp::Request req;
    req.head.insert(pair<string,string>{"Host",topUrl});
    req.askPath=endUrl;
    req.method="GET";
    req.version="HTTP/1.1";
    char buffer[500]={0},rec[5000]={0};
    http.createAskRequest(req,buffer,500);
    if(port!=443)
    {
        if(false==client.tryConnect())
            return -1;
        if(0>client.sendHost(buffer,strlen(buffer)))
            return -1;
        client.receiveHost(rec,5000);
        return 0;
    }
    if(false==client.tryConnectSSL())
        return -1;
    if(0>client.sendHostSSL(buffer,strlen(buffer)))
        return -1;
    client.receiveHostSSL(rec,5000);
    printf("%s\n\n",rec);
    return 0;
}
```

- 支持https的客户端连接

#### 静态和删除路径

```cpp
    HttpServer server(5200,true);
    server.loadStatic("/file/index.html","index.html");
    server.loadStaticFS("/file","test");
    server.deletePath("test");
```

## 速度测试

- 使用http_load测试,每秒1000个连接,持续15s

- 开启日志状态下

- 比较对象为nginx,go的gin框架

```shell
nginx server

1833 fetches, 1000 max parallel, 1.1218e+06 bytes, in 15 seconds
612 mean bytes/connection
122.2 fetches/sec, 74786.4 bytes/sec
msecs/connect: 248.187 mean, 14454.9 max, 21.731 min
msecs/first-response: 349.369 mean, 13968.4 max, 21.556 min
HTTP response codes:
  code 200 -- 1833

gin server

1528 fetches, 1000 max parallel, 921384 bytes, in 15.0002 seconds
603 mean bytes/connection
101.865 fetches/sec, 61424.9 bytes/sec
msecs/connect: 731.15 mean, 11533.5 max, 21.645 min
msecs/first-response: 319.711 mean, 7317.47 max, 21.518 min
HTTP response codes:
  code 200 -- 1528

my server

1792 fetches, 1000 max parallel, 1.1433e+06 bytes, in 15.0001 seconds
638 mean bytes/connection
119.466 fetches/sec, 76219.1 bytes/sec
msecs/connect: 143.062 mean, 7316.08 max, 21.651 min
msecs/first-response: 334.025 mean, 13735.1 max, 21.481 min
HTTP response codes:
  code 200 -- 1792
```

## 更多文档

- 详情见doc目录

## 感谢支持

---

[个人官网](http://chenxuanweb.top)
[个人简历](http://chenxuanweb.top/resume.html)
[gitee](https://gitee.com/chenxuan520/server-for-static-web)
[github](https://github.com/chenxuan520/cppweb)

---

如果你喜欢这个项目,可以给一个⭐
