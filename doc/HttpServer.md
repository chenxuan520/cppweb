## 本文目的

- 介绍httpserver类的函数和使用
  
  ## 类说明
1. 强烈建议您先看这个类这个是2.0版本的server,入门非常简单
2. 该类private继承ServerTcpIp,但您不需要提前学其父类 
   
   ## 类定义类型
   
   ### RouteType
   
   ```
   enum RouteType{
        ONEWAY,WILD,STATIC,
    };
   ```
- ONEWAY表示单解析 ,解析为该内容的报文
- WILD 表示泛解析 解析该前缀所有报文 
- STATIC是内部调用的方式，外部调用不生效
  > 如/root oneway只解析/root,wild解析/root/* 
  > wild解析务必以/结尾,oneway解析尽量不以/结尾 ,除了 / 的解析 
  
  ### AskType
  
  ```
    enum AskType{
        GET,POST,ALL,
    };
  ```
- 申请报文的类型 
- GET为get请求 ，POST为post请求，ALL表示所有请求都注册 

## 函数介绍

### 构造函数

```
HttpServer(unsigned port,bool debug=false);
```

- 第一个参数是指定绑定的端口号 
- 第二个是调试模式是否开启
  
  > 调试模式会输出请求的内容和结果
  > 关闭者不会输出 (默认关闭)
  
  ### clientOutHandle
  
  ```
  bool clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port));
  ```
- 这是处理用户断开连接的类 （非必要函数）
- 每次用户断开会触发
- pfunc是回调函数 ,回调会通过HttpServer传*this ,num传socket,ip传ip,port传端口 
- 函数只能调用一次 ，二次调用返回false
  
  ### clientInHandle
  
  ```
  bool clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port));
  ```
- 客户端连接触发函数 
- 和上一个函数类似，不做赘述
  
  ### **routeHandle**
  
  ```
  bool routeHandle(AskType ask,RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
  ```
- 核心函数 ，提供路由处理的回调 ,数量不限 
- 第一个参数enum是注册请求的类型
- 第二个是路由注册类型
- 第三个是回调函数
- 回调函数会传入Dealhttp对象,该类介绍点击 [这里](), 第二个传入*this,第三个传入发起该请求对应的socket，第四个传入 ，发送缓冲区的指针 ，在此处构造发送报文 ，第五个是发送的长度 ，需和报文同步，该函数调用后会自动发送缓冲区报文
- 返回值表示是否成功 
  
  ### **run**
  
  ```
  void run(unsigned int memory,unsigned int recBufLenChar,const char* defaultFile);
  ```
- 核心函数,调用此函数服务器开始运行
- 第一个参数是发送缓冲区大小单位是**M**(重要)
- 第二个参数是接受缓冲区大小单位是**char**
- 第三个参数是默认的网页首页
- 该函数**不会返回**,除非出错 
  
  ### httpSend
  
  ```
  int httpSend(int num,void* buffer,int sendLen);
  ```
- 额外的发送函数 ,一般不会调用,除非有额外数据发送 
- 第一个参数为socket的值 ,第二个为发送内容缓冲区 ,第三个为发送长度
  
  ### recText
  
  ```
  inline void* recText()
  ```
- 获取接受内容 
- 返回值指向报文的指针 
  
  ### recLen
  
  ```
  inline int recLen()
  ```
- 获取接受长度 
- 返回值为长度 
  
  ### lastError
  
  ```
  const char* lastError()
  ```
- 获取上一次的错误 
  
  ### disconnect
  
  ```
  inline bool disconnect(int soc)
  ```
- 主动断开某个客户端连接
- 参数为socket客户端
  ### get post all
  ```
  	bool get(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
	bool post(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
	bool all(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
  ```
- 函数是routehandle的简化版
- 除了自动生成第一个参数外其他是一样的
  ### loadStatic
  ```
  	bool loadStatic(const char* route,const char* staticPath);
  ```
- 路由路径替换，加载静态文件
- 第一个是注册的路由，第二个是映射的路径
  ## 使用例子
- 见example内的HttpServer.cpp
