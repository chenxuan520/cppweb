## https服务器安装方法



### 前提

1. 安装好openssl

2. 拥有https证书(openssl自己签发也可以)

### 自签发证书

- 使用shell

- 如果有证书可以忽略

```shell
openssl genrsa -des3 -out privkey.pem 2048 
openssl req -new -key privkey.pem -out cert.csr 
openssl req -new -x509 -key privkey.pem -out cacert.pem -days 1095
```

### 使用方法

1. 在包含头文件之前
   
   > #define CPPWEB_OPENSSL

2. 在初始化HttpServer之后加载证书
   
   > 具体见main.cpp
