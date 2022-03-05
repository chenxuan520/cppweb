## 本文目的

- 介绍DealHttp用法
- 这是重点**类**,学习完这个和HttpServer类之后就可以创作了 

## 类定义类型

### FileKind

```
enum FileKind{
   UNKNOWN=0,HTML=1,TXT=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP=7,JSON=8,
    };
```

- 文件的类型 ,报文的类型 
- UNKNOWN表示未知类型但是也可以生成报文
- NOFOUND会发送404报文 

### Status

```cpp
    enum Status{
        STATUSOK=200,STATUSNOCON=204,STATUSMOVED=301,STATUSBADREQUEST=400,STATUSFORBIDDEN=403,
        STATUSNOFOUND=404,STATUSNOIMPLEMENT=501,
    };
```

- http状态码

### Datagram

```cpp
    struct Datagram{
        Status statusCode;
        FileKind typeFile;
        unsigned fileLen;
        std::unordered_map<std::string,std::string> head;
        std::unordered_map<std::string,std::string> cookie;
        std::string typeName;
        std::string body;
        Datagram():statusCode(STATUSOK),typeFile(TXT),fileLen(0){};
    };
```

- 响应报文的结构体

- typename自定义类型,一般不用

### Request

```cpp
    struct Request{
        std::string method;
        std::string askPath;
        std::string version;
        std::unordered_map<std::string,std::string> head;
        const char* body;
        Request():body(NULL){};
    };
```

- 请求报文结构体

## 函数介绍

### analysisHttpAsk

```
const char* analysisHttpAsk(void* pask,const char* pneed="GET",int needLen=3);
```

- 获取路由值
- 在2.0很少用，一般在1.0需要手动获取

### findBackString

```
char* findBackString(char* local,int len,char* word,int maxWordLen);
```

- 获得指定字符串后面的字符串
- 第一参数是匹配字符串
- 第二参数是匹配的长度
- 第三参数是后面字符串缓冲区
- 第四是缓冲区长度
- 返回值是字符串首地址
- 2.0很少用

### createTop

```
void createTop(FileKind kind,char* ptop,int* topLen,int fileLen);
```

- 2.0很少用，创建报文头，一般直接createsendmeg

### createSendMsg

```
bool createSendMsg(FileKind kind,char* buffer,const char* pfile,int* plong)
```

- 创建报文，第一个参数是报文类型，如果没有 请注册选择unknow，如果发送404选择nofound,第二个参数是报文缓冲区，第三个参数是报文主体的文件名(主体需先写入一个文件)
  第四个参数是指针接受报文的长度
- 返回值是创建是否成功

### autoAnalysisGet

```
int autoAnalysisGet(const char* message,char* psend,const char* pfirstFile,int* plen);
```

- 在2.0不需要自己调用，是自动分析get请求生成报文函数
- 返回值是2表示文件找不到，0表示函数失败，1表示成功

### getKeyValue

```
const char* getKeyValue(const void* message,const char* key,char* value,int maxValueLen);
```

- 获取键值对，返回指向值指针
- 第一个参数是报文内容，第二个是键，第三个是只缓冲区，第四个是缓冲区长度

### getKeyLine

```
const char* getKeyLine(const void* message,const char* key,char* line,int maxLineLen);
```

- 和上一个函数类似，不过是返回一行内容

### getAskRoute

```
const char* getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen);
```

- 个人尽量不要调用是一个内部调用函数

### getRouteValue

```
const char* getRouteValue(const void* routeMeg,const char* key,char* value,unsigned int valueLen);
```

- 获取get请求路由中的键值对
- 第一个参数是路由信息，第二个是键，第三个是值缓冲区，第四个是长度
- 返回值指向值，如果找不到返回null

### getWildUrl

```
const char* getWildUrl(const void* getText,const char* route,char* buffer,int maxLen)
```

- 获取路由中携带的后续信息
- 第一个是报文，第二个注册的路由，第三个是缓冲区，第四个缓冲区大小
- 返回值指向缓冲区，失败返回null

### getRecFile

```
int getRecFile(const void* message,char* fileName,int nameLen,char* buffer,int bufferLen);
```

- 从报文中获取上传文件的内容
- 

### dealUrl

```
static void dealUrl(const char* url,char* urlTop,char* urlEnd);
```

- 处理url的静态函数，top返回url包含的域名，end.返回附加信息
- 一般不会调用

### customizeAddTop

```
void* customizeAddTop(void* buffer,int bufferLen,int statusNum,int contentLen,const char* contentType="application/json",const char* connection="keep-alive");
```

- 自己创建报文
- 第一个函数是报文的缓冲区，第二个是缓冲区长度，第四个是状态码，第四个是报文主体的长度，第四个是内容默认是json，最后一个是连接状态默认长连接
- 返回值是返回缓冲区头部

### customizeAddHead

```
void* customizeAddHead(void* buffer,int bufferLen,const char* key,const char* value);
```

- 加自定义的额外头部
- 后两个参数是键值对

### customizeAddBody

```
int customizeAddBody(void* buffer,int bufferLen,const char* body,unsigned int bodyLen);
```

- 添加自定义的body部分
- 返回值是报文总长度
- 参数最后一个是body的长度

### urlDecode

```
static const char* urlDecode(char* srcString);
```

- 解决无法显示中文的问题
- srcString是传入的字符串，会在源字符串上更改

### setCookie

```
bool setCookie(void* buffer,int bufferLen,const char* key,const char* value,int liveTime=-1,const char* path=NULL,const char* domain=NULL);
```

- 设置cookie，函数只能用于自定义报文使用
- 参数是一个键值对，liveTime是cookie生效时间，负数表示关闭浏览器失效，正数表示秒数，0表示立刻失效
- path和domain可以自己在网上查。。。

## 使用例子

- 在example中有使用实例
