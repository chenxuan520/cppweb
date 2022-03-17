## Purpose of this article

- Introduce the functions and usage of the httpserver class
  
  ## class description
1. It is strongly recommended that you look at this class first. This is the 2.0 version of the server. Getting started is very simple.

2. This class private inherits ServerTcpIp, but you do not need to learn its parent class in advance
   
   ## class definition type
   
   ### RouteType
   
   ```cpp
       enum RouteType{//oneway stand for like /hahah,wild if /hahah/*,static is recource static
           ONEWAY,WILD,STATIC,STAWILD
       };
   ```
- ONEWAY means single parsing, parsed as the message of the content

- WILD means PAN parses all packets of this prefix

- STATIC and STAWILD are the methods of internal calls, and external calls do not take effect
  
  > the struct is private
  
  ### AskType
  
  ```cpp
  enum AskType{//different ask ways in http
          GET,POST,PUT,DELETE,OPTIONS,CONNECT,ALL,
      };
  ```

- Type of application message

- GET is a get request, POST is a post request, ALL means all requests are registered, and so on

### RunModel

```cpp
enum RunModel{//the server model of run
    FORK,MULTIPLEXING,THREAD
};
```

- The mode in which the server is running

## Function introduction

### Constructor

```cpp
HttpServer(unsigned port, bool debug=false, RunModel serverModel=MULTIPLEXING, unsigned threadNum=5)
    :ServerTcpIp(port),model(serverModel)
```

- The first parameter is the port number to specify the binding

- The second is whether the debug mode is turned on
  
  > Debug mode will output the content of the request and the result
  > Closed will not output (default closed)

- The third is server mode

- The fourth is the number of threads if multi-threaded

### clientOutHandle

```cpp
bool clientOutHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port));
```

- This is the class that handles user disconnection (non-essential function)
- Triggered every time user disconnects
- pfunc is a callback function, the callback will pass *this through HttpServer, num pass socket, ip pass ip, port pass port
- The function can only be called once, and the second call returns false

### clientInHandle

```cpp
bool clientInHandle(void (*pfunc)(HttpServer&,int num,void* ip,int port));
```

- Client connection trigger function
- Similar to the previous function, so I won't go into details

### **routeHandle**

```cpp
bool routeHandle(AskType ask,const char* route,void (*pfunc)(HttpServer&,DealHttp&,int))
```

- Core function, providing callbacks for routing processing, the number is not limited
- The first is the route registration type
- The second parameter is the registered route
- The third is the callback function
- The callback function will pass in the Dealhttp object. Click [here](./DealHttp.md) for the introduction of this class. The first pass in *this, and the third pass in the socket corresponding to the request. After this function is called, it will automatically send buffer message
- The return value indicates whether it was successful or not

### **run**

```cpp
void run(const char* defaultFile=NULL)
```

- Core function, call this function to start the server
- The first parameter is the default home page
- This function **will not return** unless an error occurs

### httpSend

```cpp
int httpSend(int num, void* buffer, int sendLen);
```

- Extra send function, generally not called unless additional data is sent
- The first parameter is the value of the socket, the second is the send content buffer, and the third is the send length

### recText

```cpp
inline void* recText()
```

- Get accepted content
- Returns a pointer to the message

### recLen

```cpp
inline int recLen()
```

- get accepted length
- the return value is the length

### lastError

```cpp
const char* lastError()
```

- get the last error

### disconnect

```cpp
inline bool disconnect(int soc)
```

- Actively disconnect a client connection
- parameter is socket client

### get post all

```cpp
bool get(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
  bool post(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
  bool all(RouteType type,const char* route,void (*pfunc)(DealHttp&,HttpServer&,int,void*,int&));
```

- The function is a simplified version of routehandle
- the same except that the first parameter is automatically generated

### loadStatic

```cpp
    bool loadStatic(const char* route,const char* staticPath);
```

- Route path replacement, load static files

- The first is the registered route, the second is the mapped path

- also accepts * as a pan match
  
  ## Usage example

- see HttpServer.cpp in the example

### deletePath

```cpp
bool deletePath(const char* path);
```

- Set forbidden access paths
- parameter is a forbidden path

### changeSetting

```cpp
void changeSetting(bool debug,bool isLongCon,bool isForkModel);
```

- Modify server configuration
- One parameter is whether to enable debug mode
- The second parameter is whether to use http long connection
- The three parameters are whether to use multi-process mode

### setMiddleware

```cpp
bool setMiddleware(void (*pfunc)(HttpServer&,DealHttp&,int))
```

- Set up middleware, the function will be called when there is a message

### continueNext

```cpp
inline void continueNext(int cliSock)
```

- Continue execution after setting middleware

### setLog

```cpp
bool setLog(void (*pfunc)(const void*,int),void (*errorFunc)(const void*,int))
```

- Set the log, the first one is the access log

- The second is the error log

### getCompleteMessage

```cpp
int getCompleteMessage(int sockCli)
```

- Get the complete message

### changeSetting

```cpp
void changeSetting(bool debug,bool isLongCon,bool isAuto=true,unsigned sendLen=1)
```

- Change the settings, the first is whether to debug mode, the second is whether to long link, the third is whether to enable the default path recognition, the fourth is the send buffer size, the unit is **M**

### stopServer

```cpp
inline void stopServer()
```

- stop the server from running
- the run function will end
