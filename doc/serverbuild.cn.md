## 本文目标

- 使用cppweb搭建简单的http服务器和https服务器

## 脚本安装教程

> linux可用

1. git clone https://gitee.com/chenxuan520/server-for-static-web.git

2. ./install.sh

3. 填入端口等信息

4. 服务器运行 
   
##### 填入信息

5. 填入绑定的端口(如5200)

6. 填入网页默认的html文件名(index.html)

7. 填入是否运行在后台 (true or false)

8. 填入是否使用守护进程 (true or false)

## 修改配置法(推荐)

> windows和linux都可

1. git clone https://gitee.com/chenxuan520/server-for-static-web.git

2. make (windows下make -f makefile.win)
   
   > 如果有https证书也可以make ssl

3. 修改config.json,[介绍](./configjson.cn.md)

4. ./main

## 信息说明

- 不运行在后台的话shell结束会一起结束

- 守护进程可以保证宕机快速重启 

## 修改配置

- 直接修改config.json

- ./main --reload

## 停止服务器

- ./main --stop

## 指定配置文件

- ./main --config=(文件名字)

## 查看帮助

- ./main --help

## 搭建https服务

### 说明

1. 用于修改配置法,脚本法无效

2. 需要提前安装openssl

```shell
    sudo apt install openssl libssl-dev
```

### 1. 获取证书

#### openssl自己签发(用于测试)

```shell
openssl genrsa -des3 -out privkey.pem 2048 
openssl req -new -key privkey.pem -out cert.csr 
openssl req -new -x509 -key privkey.pem -out cacert.pem -days 1095
```

#### 使用域名获取ssl证书

1. 获取路径有域名购买处或者[freessl](https://freessl.cn/)

2. 下载用于nginx的证书,包含一个pem和一个key

3. 转换key为pem

```shell
openssl rsa -in (key name).key -out (new name).pem
```

### 2.修改config.h

- 把"cert path"和"key path"的值改为证书路径

- 如果没有密码把"cert password"这行注释

### 3. 编译运行

1. **make ssl**

2. ./main
