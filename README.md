# C++网页后台制作
## 作者
**chenxuan**

![dog](https://chenxuan520.oschina.io/chenxuanweb/pic/untitled1.png)
## 制作目的
*为了使用C++搭建轻量级网页*
## 特点说明
- 支持windows以及linux
- 支持get以及post方法
- 支持自定义扩展后台 通过dealaskcpp来快速扩展网页内容
- 支持mysql数据库
- 支持扩展使用redis数据库
- 支持扩展使用线程池服务器
- 自带守护进程，宕机恢复时间小于1分钟
## 使用教程
1. git clone https://gitee.com/chenxuan520/server-for-static-web.git
2. chmod 777 ./install.sh
3. ./install.sh
4. 填入端口等信息
5. 看到*the server is ok*表示成功
> *或者*(较麻烦不推荐)
1. git clone https://gitee.com/chenxuan520/server-for-static-web.git
2. chmod 777 ./init ./main
3. ./init 
4. 打开8888端口远程配置
5. ./main
## 制作说明
- windows使用select模型
- linux使用epoll模型
- 未使用任何框架，纯本人手写
- 使用的所有类**具有跨平台性**
## 联系作者
+ 任何问题1607772321@qq.com邮箱联系 
---
*如果你觉得不错可以给个⭐*

[个人网页](http://chenxuanweb.top) [gitee主页](https://gitee.com/chenxuan520)
