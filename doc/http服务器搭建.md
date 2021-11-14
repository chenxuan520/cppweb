## 本文目标
- 使用cppweb搭建简单的http服务器
## 安装教程
1. git clone https://gitee.com/chenxuan520/server-for-static-web.git
2. chmod 744 ./install.sh
3. ./install.sh
4. 填入端口等信息
5. 服务器运行 
## 填入信息
1. 填入使用的版本 (1或2)(区别请见[这里]() )  
2. 填入绑定的端口(如5200)
3. 填入网页默认的html文件名(index.html)
4. 填入是否使用mysql数据库 (y or n)
5. 填入是否运行在后台 (y or n)
6. 填入分配给程序的内存(单位为M)
7. 填入是否使用守护进程 (y or n)
## 信息说明
- 数据库需先安装mysql的库
- 不运行在后台的话shell结束会一起结束
- 守护进程可以保证宕机快速重启 
## 修改配置
- 在install.sh运行完后会形成my.ini文件在同一个目录下
- 内容分别是端口,默认网页,是否运行在后台
- 修改后重启程序即可