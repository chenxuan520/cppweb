## 本文目标

- 使用cppweb搭建简单的http服务器
  
  ## 安装教程
1. git clone https://gitee.com/chenxuan520/server-for-static-web.git

2. ./install.sh

3. 填入端口等信息

4. 服务器运行 
   
   ## 填入信息

5. 填入绑定的端口(如5200)

6. 填入网页默认的html文件名(index.html)

7. 填入是否运行在后台 (true or false)

8. 填入是否使用守护进程 (true or false)
   
   ## 信息说明
- 数据库需先安装mysql的库

- 不运行在后台的话shell结束会一起结束

- 守护进程可以保证宕机快速重启 
  
  ## 修改配置

- 在install.sh运行完后会形成config.json文件在同一个目录下

- 内容分别是端口,默认网页,是否运行在后台等

- 修改后重启程序即可