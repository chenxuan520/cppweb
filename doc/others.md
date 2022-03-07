## 其他类说明

### ClientTcpIp

- 该类为客户端类,一般用不到

- 支持ssl,需#define CPPHTTPLIB_OPENSSL_SUPPORT

#### 构造函数

```cpp
	ClientTcpIp(const char* hostIp,unsigned short port)
```

- 第一个参数为连接ip,第二个为连接的端口

- 如果暂时未知填NULL

#### addhostip

```cpp
void addHostIp(const char* ip,unsigned short port=0)
```

- 初始化后添加或者更改ip和端口

#### 其他函数类似就不介绍
