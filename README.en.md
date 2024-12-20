# Attention: cppweb has been refactored and upgraded to [cppnet](https://github.com/chenxuan520/cppnet), and this repository will soon be discontinued.

- The cppweb code history is relatively heavy, and the implementation is not elegant enough, without automated testing and other issues, so it took a little time to refactor it into cppnet.

# cppweb framework introduction

#### Introduction in [English](./README.en.md)

## author

**chenxuan**

![dog](http://cdn.androidftp.top/pic/chenxuanweb/dog.png)

## Original intention

When I was building a C++ server, I found that the boost library is too large and not friendly to novices, and libevent is not friendly to newcomers when it is applied to specialization. It is very cumbersome to develop the http backend with C++ and the code reuse rate is low. When using the gin framework of go At that time, I came up with the idea of ​​writing a similar back-end lightweight library and simple server.

## Official website

[https://chenxuanweb.top/cppweb](https://chenxuanweb.top/cppweb)

## Framework Features

- With Linux and windows cross-platform features
  
  > Cross-Platform Instructions
  > 
  > Supports clang and MinGW under the windows platform, does not support mscv (~~because the author does not like~~)
  > 
  > Support clang and gcc on linux platform
  > 
  > Not tested on the mac platform (~~because the author does not have a mac~~)

- Good scalability, free custom processing of http responses

- Easy to install, the core only has a header file hpp/cppweb.h

- Low learning cost, you can master the usage in half an hour

- Detailed documentation in Chinese, all code is open source

- Lightweight, the core code has only one header file, the code contains 5000 lines of comments, and can be directly operated using the header file

## project structure

- cppweb.h in hpp is all code, including this header file can be used

- hpp cpp is the project source file directory

- example is an example of use, it is recommended to read first

- doc is a description document, including the usage of functions and classes

- class is compiled for some classes, including a simple memory leak detection header file

- install.sh runs the script for the server

- The old folder is the header and source files of version 1.0, which is not recommended

## Project Features

1. Written in C++, with faster running speed

2. Support the use of custom middleware

3. Support cookie generation and reading

4. Contains a [json parsing and formatting library] (https://gitee.com/chenxuan520/cppjson) (also written by the author)

5. Contains the email class, which can be called to send emails

6. Support HTML rendering

7. Support for managing requests through routing and routing groups
   
   > Support lam expressions

8. Support log generation and implementation, the log system is about 300,000 per second

9. With io multiplexing, multi-process, thread pool, reactor four modes
   
   > io multiplexing supports epoll (epoll only supports linux) and select models

10. Contains thread pools and thread pool servers

11. Support https connection
    
    > Including server and client

12. Comes with a server written by the framework
    
    1. Support route 301 forwarding
    
    2. Support reverse proxy, load balancing
    
    > Load balancing supports four methods: round robin, random, hash, etc.
    
    3. Support path replacement
    
    4. High degree of customization through json configuration
    
    5. With its own daemon and background operation, it can restart quickly when downtime

## project instruction

- This project can be used for individuals to build small website backend services and load balancing
- I'm only a sophomore student. I urge you to send me the inadequacies of the framework through issue and I will definitely improve them.
- If the code is not enough, please contact me to improve it
- Please read the documentation under doc before using the project

## Build the server

- Under linux, ./install.sh is the installation script, you can install it by running
- [doc introduction](./doc/serverbuild.md)

## Framework introduction

#### server running

- [doc document](./doc/serverbuild.md)

#### Install the framework

- [doc introduction](./doc/framework.md)

#### Simple to use

```cpp
#include "../../hpp/cppweb.h"//include header file
using namespace cppweb;
int main()
{
    HttpServer server(5200,true);//Enter the running port and whether to enable the printing debug mode
    server.run("index.html");//The default file when the input access path is /, if there is none, you can leave it blank
    return 0; //If there is no error, it will always run, unless there is an error, you can use lastError to get the error
}
```

- HttpServer class [introduction](./doc/HttpServer.en.md)

#### Build https

```cpp
#define CPPWEB_OPENSSL
#include "../../hpp/cppweb.h"
using namespace cppweb;
int main()
{
    HttpServer server(5201);
    server.loadKeyCert("./cacert.pem","./privkey.pem","123456");
    //The first is the certificate location, the second is the private key location, and the third is the password (if there is none, you can leave it blank)
    server.run("./index.html");
    return 0;
}
```

- need to install openssl

- Need to define macro before including header file

#### Routing settings

```cpp
    server.get("/lam*",[](HttpServer&,DealHttp& http,int){
        http.gram.body="text";//Set the message content
    });
    server.get("/",{login,message});//Multiple processing functions
```

- Supports lam expressions and normal functions

- *Extensions* to support routing

- Supports sequential processing of multiple functions like gin, enclosed in parentheses

#### Variable settings

```cpp
structTemp{
    int soc;
    char cookie[128];
};
void login(HttpServer&,DealHttp& http,int soc){
    auto flag=http.req.getCookie("key",http.info.recText);
    if(flag.size()>0){
        Temp temp={soc,{0}};
        strncpy(temp.cookie,flag.c_str(),128);
        http.setVar<Temp>("cookie",temp);//You can place anything through the template
        http.info.isContinue=true;
    }else{
        http.gram.json(DealHttp::STATUSOK,Json::createJson({{"status","find wrong"}}));
    }
}
void message(HttpServer&,DealHttp& http,int){
    Temp* pstr=(Temp*)http.getVar("cookie");
    if(pstr==NULL){
        http.gram.json(DealHttp::STATUSNOFOUND,Json::createJson({{"status","wrong"}}));
    }else{
        http.gram.json(DealHttp::STATUSOK,Json::createJson({{"cookie",pstr->cookie},{"soc",pstr->soc}}));
    }
}
```

- The type of the set variable is preferably the C type, and try not to have a constructor

#### html rendering

```cpp
void func(HttpServer&,DealHttp&http,int)
{
    unordered_map<string,string> hash;
    hash["name"]="chenxuan";
    hash["lover"]="xiaozhu";
    http.gram.body=FileGet::renderHtml("./test.html",hash);
    http.gram.typeFile=DealHttp::HTML;
}
int main()
{
    HttpServer server(5200, true);
    auto group=server.createGroup("/temp");
    group.get("/",func);
    server.run();
    return 0;
}
```

- Generate html by regular search and replace

- The default format is {{.name}}, and the left and right borders can be customized

#### Middleware settings

```cpp
void pfunc(HttpServer& server, DealHttp& http, int soc)
{
    http.head["try"]="en";
    server.continueNext(soc);//Continue to perform the following default operations, or not
}
int main()
{
    HttpServer server(5200, true);
    server.setMiddleware(pfunc);//Set middleware
    server.get("/root",[](HttpServer&,DealHttp& http,int){
               http.gram.body="text";
               });
    server.run("./index.html");
    return 0;
}
```

- Support middleware to uniformly process messages
- Support for multiple middleware

#### log settings

```cpp
    server.setLog(LogSystem::recordRequest,NULL);
    server.get("/stop",[](HttpServer& server,DealHttp&,int){
               server.stopServer();
               });
```

- LogSystem has some templates, you can also set it yourself

#### Parse the message

```cpp
void func(HttpServer& server, DealHttp& http, int)
{
    DealHttp::Request req;
    http.req.analysisRequest(http.info.recText);
    printf("new:%s %s %s\n",http.req.method.c_str(),http.req.askPath.c_str(),req.version.c_str());
    for(auto iter=http.req.head.begin();iter!=http.req.head.end();iter++)
        printf("%s:%s\n",iter->first.c_str(),iter->second.c_str());
    printf("body:%s\n",http.req.body);
    http.gram.statusCode=DealHttp::STATUSOK;
    http.gram.typeFile=DealHttp::JSON;
    http.gram.body="{\"ha\":\"ha\"}";
}
int main()
{
    HttpServer server(5200,true);//input the port bound
    server.all("/root",func);
    server.run("./index.html");
    return 0;
}
```

- Parse through http built-in structure
- DealHttp class [introduction](./doc/DealHttp.en.md)

#### Get route key value

```cpp
    DealHttp::Request req;
    unordered_map<string,string> tree;
    req.routePairing("/try/:id/:name",tree);
    Json json={
        {"id",tree["id"]},
        {"name",tree["name"]}
    };
```

> via: split key
> the result is stored in the second parameter

#### set cookies

```cpp
void cookie(HttpServer& server, DealHttp& http, int)
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
        {"key",buffer},
        {"status","ok"}
    };
    http.gram.body=json();
    http.gram.typeFile=DealHttp::JSON;
}
int main()
{
    HttpServer server(5200, true);
    server.get("/cookie",cookie);
    server.run("index.html");
    return 0;
}
```

#### Client Creation

```cpp
    ClientTcpIp client(ip,port);
    if(client.lastError()!=NULL)
    {
        printf("error%s\n;",client.lastError());
        exit(0);
    }
    DealHttp http;
    DealHttp::Request req;
    req.head.insert(pair<string,string>{"Host",topUrl});
    req.askPath=endUrl;
    req.method="GET";
    req.version="HTTP/1.1";
    char buffer[500]={0},rec[5000]={0};
    http.createAskRequest(req,buffer,500);
    if(!isHttps)
    {
        if(false==client.tryConnect())
        {
            printf("connect wrong\n");
            exit(0);
        }
        if(0>client.sendHost(buffer,strlen(buffer)))
        {
            printf("%d",errno);
            exit(0);
        }
        client.receiveHost(rec,5000);
    }
    else
    {
        if(false==client.tryConnectSSL())
        {
            printf("connect wrong\n");
            exit(0);
        }
        if(0>client.sendHostSSL(buffer,strlen(buffer)))
        {
            printf("%d",errno);
            exit(0);
        }
        client.receiveHostSSL(rec,5000);
    }
```

- Client connections that support https
- For details, see the example under example

#### Static and delete paths

```cpp
    HttpServer server(5200, true);
    server.loadStatic("/file/index.html","index.html");
    server.loadStatic("/try/*","test");
    server.deletePath("test");
```

## Speed ​​test

- Tested with http_load, 1000 connections per second for 15s

- When the log is turned on

- The comparison object is the gin framework of nginx and go

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

- Four modes speed test

```shell
reactor
1419 fetches, 100 max parallel, 905322 bytes, in 10 seconds
638 mean bytes/connection
141.9 fetches/sec, 90532.2 bytes/sec
msecs/connect: 30.5315 mean, 1048.08 max, 12.051 min
msecs/first-response: 160.906 mean, 7268.44 max, 13.543 min
HTTP response codes:
  code 200 -- 1419

IO
1415 fetches, 100 max parallel, 902770 bytes, in 10 seconds
638 mean bytes/connection
141.5 fetches/sec, 90277 bytes/sec
msecs/connect: 38.5241 mean, 3052.13 max, 12.052 min
msecs/first-response: 189.096 mean, 7839.18 max, 13.543 min
HTTP response codes:
  code 200 -- 1415

thread
1405 fetches, 100 max parallel, 894476 bytes, in 10 seconds
636.638 mean bytes/connection
140.5 fetches/sec, 89447.6 bytes/sec
msecs/connect: 40.6892 mean, 1046.36 max, 12.054 min
msecs/first-response: 207.844 mean, 8081.35 max, 13.576 min
3 bad byte counts
HTTP response codes:
  code 200 -- 1402

fork
1410 fetches, 100 max parallel, 899580 bytes, in 10 seconds
638 mean bytes/connection
141 fetches/sec, 89958 bytes/sec
msecs/connect: 37.9042 mean, 2035.83 max, 12.043 min
msecs/first-response: 178.578 mean, 7550.83 max, 13.764 min
HTTP response codes:
  code 200 -- 1410
```

## more documentation

- See the doc directory for details

## Thanks for the support

**logo**

![logo](file:///home/chaiquan/code/gitee/server-for-static-web/logo.png?msec=1656509528131)

---

[Personal official website](http://chenxuanweb.top)
[Resume](http://chenxuanweb.top/resume.html)

[gitee](https://gitee.com/chenxuan520/server-for-static-web)
[github](https://github.com/chenxuan520/cppweb)

(Personal websites all run on this framework)

---

If you like this project, you can give a ⭐
