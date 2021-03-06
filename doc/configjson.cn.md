## 配置文件说明

- 服务器通过读取config.json配置文件运行,请确保服务器目录有此文件

- 配置文件config.json使用json格式支持//和/**/作为注释

### port

- 绑定的端口

- 若为0则随即绑定

### defalut file

- 默认访问的html文件
- 如果不想设置可为""

### model

- 服务器运行模式

- 只能为"MULTIPLEXING" or "FORK" or"THREAD" or "REACTOR"

### long connect

- bool类型,是否开启长连接

### background

- bool类型,是否运行在后台

### guard

- 是否开启宕机自动重启

### logger

- 是否开启日志

### log path

- 日志储存路径

### auto

- 是否开启路径自动识别

### message print

- bool类型,是否把输出打印到终端

### rediect

- 重定向,结构为数组

```json
        "rediect":[
         {"path":"/root","redirect":"https://baidu.com"},
         {"path":"/temp","redirect":"./tem"}
        ],
```

### delete path

- 手动排除路径

- 为字符串数组

### replace

- 替换路径

```json
        "replace":[
         {"path":"/root","repalce":"./template"},
         {"path":"/temp*","repalce":"./tem"}
        ],
```

### proxy

- 设置代理,以及j负载均衡,结构为对象数组

- 每个成员包括三个成分
  
  1. model 模式只能从RANDOM,HASH,POLLING,POLLRAN选择
     
     > random是按比重随机
     > 
     > hash是通过哈希分配
     > 
     > polling按比重轮询
     > 
     > pollran轮询加随机
  
  2. path为转发的路径,支持*匹配
  
  3. host为服务器数组,格式为ip:port
  
  4. weight为权重数组,数量应与host相同否则会出错

```json
    //set agent array
     "reverse proxy":[
     {"model":"RANDOM","path":"/try*","host":["127.0.0.1:5201"],"weight":[1]}
     ],
```

### 可选

#### cert path

- https cert位置

#### key path

- https key位置

#### cert password

- key密码(没有直接删除该行)

#### memory

- int类型,没有设置默认为1

- 作用为设置初始发送缓冲区大小,单位为M

#### thread num

- 指定线程数量,int类型,默认为5

- 只有在THREAD模式下才有效
