## 本文目的

- 介绍httpserver类的函数和使用
  
  ## 类说明
1. 强烈建议您先看这个类这个是2.0版本的server,入门非常简单

2. 该类private继承ServerTcpIp,但您不需要提前学其父类 
   
   ## 类定义类型
   
   ### RouteType
   
   ```cpp
       enum RouteType{//oneway stand for like /hahah,wild if /hahah/*,static is recource static
           ONEWAY,WILD,STATIC,STAWILD
       };
   ```
- ONEWAY表示单解析 ,解析为该内容的报文

- WILD 表示泛解析 解析该前缀所有报文 

- STATIC和STAWILD是内部调用的方式，外部调用不生效
  
  > 该结构体私有
  
  ### AskType
  
  ```cpp
      enum AskType{//different ask ways in http
          GET,POST,PUT,DELETE,OPTIONS,CONNECT,ALL,
      };
  ```

- 申请报文的类型 

- GET为get请求 ，POST为post请求，ALL表示所有请求都注册,以此类推

### RunModel

```cpp
enum RunModel{//the server model of run
    FORK,MULTIPLEXING,THREAD
};
```

- 服务器运行的模式

## 函数介绍

### 构造函数

```cpp
HttpServer(unsigned port,bool debug=false,RunModel serverModel=MULTIPLEXING,unsigned threadNum=5)
    :ServerTcpIp(port),model(serverModel)
```

- 第一个参数是指定绑定的端口号 

- 第二个是调试模式是否开启
  
  > 调试模式会输出请求的内容和结果
  > 关闭者不会输出 (默认关闭)

- 第三个是服务器模式

- 第四个是如果多线程的话线程数量

### clientOutHandle

```cpp
bool clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port));
```

- 这是处理用户断开连接的类 （非必要函数）
- 每次用户断开会触发
- pfunc是回调函数 ,回调会通过HttpServer传*this ,num传socket,ip传ip,port传端口 
- 函数只能调用一次 ，二次调用返回false

### clientInHandle

```cpp
bool clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port));
```

- 客户端连接触发函数 
- 和上一个函数类似，不做赘述

### **routeHandle**

```cpp
bool routeHandle(AskType ask,const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
```

- 核心函数 ，提供路由处理的回调 ,数量不限 
- 第一个是路由注册类型
- 第二个参数是注册的路由
- 第三个是回调函数
- 回调函数会传入Dealhttp对象,该类介绍点击 [这里](./DealHttp.md), 第一个传入*this,第三个传入发起该请求对应的socket，该函数调用后会自动发送缓冲区报文
- 返回值表示是否成功 

### **run**

```cpp
void run(const char* defaultFile=NULL)
```

- 核心函数,调用此函数服务器开始运行
- 第一个参数是默认的网页首页
- 该函数**不会返回**,除非出错 

### httpSend

```cpp
int httpSend(int num,void* buffer,int sendLen);
```

- 额外的发送函数 ,一般不会调用,除非有额外数据发送 
- 第一个参数为socket的值 ,第二个为发送内容缓冲区 ,第三个为发送长度

### recText

```cpp
inline void* recText()
```

- 获取接受内容 
- 返回值指向报文的指针 

### recLen

```cpp
inline int recLen()
```

- 获取接受长度 
- 返回值为长度 

### lastError

```cpp
const char* lastError()
```

- 获取上一次的错误 

### disconnect

```cpp
inline bool disconnect(int soc)
```

- 主动断开某个客户端连接
- 参数为socket客户端

### get post all

```cpp
bool get(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
  bool post(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
  bool all(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
```

- 函数是routehandle的简化版
- 除了自动生成第一个参数外其他是一样的

### loadStatic

```cpp
    bool loadStatic(const char* route,const char* staticPath);
```

- 路由路径替换，加载静态文件

- 第一个是注册的路由，第二个是映射的路径

- 同样接受*作为泛匹配
  
  ## 使用例子

- 见example内的HttpServer.cpp

### deletePath

```cpp
bool deletePath(const char* path);
```

- 设置禁止访问的路径
- 参数为禁止的路径

### changeSetting

```cpp
void changeSetting(bool debug,bool isLongCon,bool isForkModel);
```

- 修改server配置
- 一参为是否开启调试模式
- 二参为是否使用http长连接
- 三参为是否使用多进程模式

### setMiddleware

```cpp
bool setMiddleware(void (*pfunc)(HttpServer&,DealHttp&,int))
```

- 设置中间件,在有报文会调用该函数

### continueNext

```cpp
inline void continueNext(int cliSock)
```

- 设置中间件之后继续执行

### setLog

```cpp
bool setLog(void (*pfunc)(const void*,int),void (*errorFunc)(const void*,int))
```

- 设置日志,第一个是访问日志

- 第二个是错误日志

### getCompleteMessage

```cpp
int getCompleteMessage(int sockCli)
```

- 获取完整报文

### changeSetting

```cpp
void changeSetting(bool debug,bool isLongCon,bool isAuto=true,unsigned maxSendLen=1)
```

- 改变设置,第一个是是否调试模式,第二个是是否长链接,第三个是是否开启路径默认识别,第四个是发送缓冲区最大大小,单位为**M**

### stopServer

```cpp
inline void stopServer()
```

- 停止服务器运行
- run函数会结束