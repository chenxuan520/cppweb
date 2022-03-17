## 本文目标

- 使用cppweb搭建简单的http服务器

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

## 修改配置法

> windows和linux都可

1. git clone https://gitee.com/chenxuan520/server-for-static-web.git

2. make (windows下make -f makefile.win)
   
   > 如果有https证书也可以make ssl

3. 修改config.json,[介绍](./配置文件说明.md)

4. ./main

## 信息说明

- 不运行在后台的话shell结束会一起结束

- 守护进程可以保证宕机快速重启 

## 修改配置

- 直接修改config.json

- ./main --reload

- 等待大约半分钟

## 停止服务器

- ./main --stop