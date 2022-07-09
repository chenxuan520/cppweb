## Purpose of this article

- Introduce DealHttp usage
- This is the key **class**, you can create after learning this and the HttpServer class

## class definition type

### FileKind

```
enum FileKind{
   UNKNOWN=0,HTML=1,TXT=2,IMAGE=3,NOFOUND=4,CSS=5,JS=6,ZIP=7,JSON=8,
    };
```

- file type, message type
- UNKNOWN means unknown type but can also generate packets
- NOFOUND will send a 404 message

### Status

```cpp
    enum Status{
        STATUSOK=200,STATUSNOCON=204,STATUSMOVED=301,STATUSBADREQUEST=400,STATUSFORBIDDEN=403,
        STATUSNOFOUND=404,STATUSNOIMPLEMENT=501,
    };
```

- http status code

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

- The structure of the response message

- typename is a custom type, generally not used

#### Datagarm functions

##### Message function

```cpp
inline void json(Status staCode,const std::string& data)
inline void html(Status staCode, const std::string& data)
inline void js(Status staCode, const std::string& data)
inline void css(Status staCode, const std::string& data)
inline void txt(Status staCode, const std::string& data)
inline void image(Status staCode, const std::string& data)
inline void file(Status staCode, const std::string& data)
```

- Parameters, status codes and data

- The function will generate the corresponding http message data

##### state function

```cpp
inline void noFound()
inline void forbidden()
void redirect(const std::string& location,bool forever=false)
```

- Corresponding to 404, 403 and redirection packets

### Request

```cpp
    class Request{
    public:
        std::string method;
        std::string askPath;
        std::string version;
        std::unordered_map<std::string,std::string> head;
        const char* body;
        const char* error;
        bool isFull;//judge if the request be analyse
}
```

- Request message structure

#### request function

##### analysisRequest

```cpp
bool analysisRequest(const void* recvText,bool onlyTop=false)
```

- Parse the request message, the first parameter is the message, whether the return is successful

- recvText is generally passed in http.info.recvText

- Request other functions need to go through this function parsing first

##### getWildUrl

```cpp
std::string getWildUrl(const char* route, void* recvMsg=NULL)
```

- Get the follow-up information carried in the route

- The first is the route, the second is the message, if it has been parsed, it can be set to empty

- return result

##### routePairing

```cpp
bool routePairing(std::string key,std::unordered_map<std::string,std::string>& pairMap,const char* recvText=NULL)
```

- route matching

- See exanple for details

##### formvalue

```cpp
std::string formValue(const std::string& key, void* buffer=NULL)
```

- Get the form value in the post message

##### routeValue

```cpp
std::string routeValue(const std::string& key, void* buffer=NULL)
```

- Get the key Value in the route

##### createAskRequest

```cpp
    bool createAskRequest(void* buffer, unsigned buffLen)
```

- Create request message

- General client use

## Function introduction

### analysisHttpAsk

```cpp
const char* analysisHttpAsk(void* pask,const char* pneed="GET",int needLen=3);
```

- Get route value
- Rarely used in 2.0, generally need to be obtained manually in 1.0

### findBackString

```
char* findBackString(char* local,int len,char* word,int maxWordLen);
const char* findBackString(char* local, int len, std::string& buffer)
```

- get the string following the specified string
- The first parameter is the matching string
- The second parameter is the length of the match
- The third parameter is the back string buffer
- Fourth is the buffer length
- The return value is the first address of the string
- 2.0 is rarely used

### createTop

```
void createTop(FileKind kind,char* ptop,int* topLen,int fileLen);
```

- 2.0 is rarely used, create a header, usually directly createsendmeg

### createSendMsg

```
bool createSendMsg(FileKind kind, char* buffer, const char* pfile, int* plong)
```

- To create a message, the first parameter is the message type. If not, please register and select unknow. If you send 404, select nofound, the second parameter is the message buffer, and the third parameter is the file name of the message body (body need to write to a file first)
  The fourth parameter is the length of the pointer to receive the message
- The return value is whether the creation was successful or not

### autoAnalysisGet

```
int autoAnalysisGet(const char* message,char* psend,const char* pfirstFile,int* plen);
```

- In 2.0, you don't need to call it yourself, it is a function that automatically analyzes the get request and generates a message
- The return value is 2 for file not found, 0 for function failure, and 1 for success

### getKeyValue

```
const char* getKeyValue(const void* message,const char* key,char* value,int maxValueLen);
```

- Get key-value pair, return pointer to value
- The first parameter is the message content, the second is the key, the third is the buffer only, and the fourth is the buffer length

### getKeyLine

```
const char* getKeyLine(const void* message,const char* key,char* line,int maxLineLen);
```

- Similar to the previous function, but returns a line of content

### getAskRoute

```
const char* getAskRoute(const void* message,const char* askWay,char* buffer,unsigned int bufferLen);
```

- Personally try not to call a function that is an internal call

### getRouteValue

```
const char* getRouteValue(const void* routeMeg,const char* key,char* value,unsigned int valueLen);
```

- Get the key-value pair in the get request route
- The first parameter is the routing information, the second is the key, the third is the value buffer, and the fourth is the length
- return value to point to value, or null if not found

### getWildUrl

```
const char* getWildUrl(const void* getText, const char* route, char* buffer, int maxLen)
```

- Get the follow-up information carried in the route
- The first is the message, the second is the registered route, the third is the buffer, and the fourth is the buffer size
- The return value points to the buffer, or null on failure

### getRecFile

```
int getRecFile(const void* message,char* fileName,int nameLen,char* buffer,int bufferLen);
```

- Get the content of the uploaded file from the message

### dealUrl

```
static void dealUrl(const char* url,char* urlTop,char* urlEnd);
```

- A static function for processing url, top returns the domain name contained in the url, end. returns additional information
- generally not called

### customizeAddTop

```
void* customizeAddTop(void* buffer,int bufferLen,int statusNum,int contentLen,const char* contentType="application/json",const char* connection="keep-alive");
```

- create the message yourself
- The first function is the buffer of the message, the second is the buffer length, the fourth is the status code, the fourth is the length of the message body, the fourth is the content default is json, and the last is the connection Status default long connectioncatch
  - The return value is the header of the return buffer
  
  ### customizeAddHead
  
  ```
  void* customizeAddHead(void* buffer,int bufferLen,const char* key,const char* value);
  ```
  
  - Add custom extra headers
  - The last two parameters are key-value pairs
  
  ### customizeAddBody
  
  ```
  int customizeAddBody(void* buffer,int bufferLen,const char* body,unsigned int bodyLen);
  ```
  
  - Add custom body section
  - The return value is the total length of the message
  - The last parameter is the length of the body
  
  ### urlDecode
  
  ```
  static const char* urlDecode(char* srcString);
  ```
  
  - Solve the problem that Chinese cannot be displayed
  - srcString is the incoming string, will be changed on the source string
  
  ### setCookie
  
  ```
  bool setCookie(void* buffer,int bufferLen,const char* key,const char* value,int liveTime=-1,const char* path=NULL,const char* domain=NULL);
  ```
  
  - Set cookies, the function can only be used for custom messages
  - The parameter is a key-value pair, liveTime is the effective time of the cookie, a negative number means closing the browser and it will expire, a positive number means seconds, and 0 means it will expire immediately
  - The path and domain can be checked online. . .
  
  ## Usage example
  
  - There are examples of use in example
