## Configuration file description

- The server runs by reading the config.json configuration file, please make sure the server directory has this file

- The configuration file config.json uses the json format to support // and /**/ as comments

### port

- bound port

- If it is 0, it will be bound immediately

### defalut file

- html files accessed by default
- If you don't want to set it can be ""

###model

- Server operating mode

- can only be "MULTIPLEXING" or "FORK" or "THREAD"

### long connect

- bool type, whether to enable long connection

### background

- bool type, whether to run in the background

### guard

- Whether to enable automatic restart when downtime

### logger

- whether to enable logging

### auto

- Whether to enable automatic path recognition

### redirect

- redirection, the structure is an array

```json
        "redirect":[
         {"path":"/root","redirect":"https://baidu.com"},
         {"path":"/temp","redirect":"./tem"}
        ],
```

### delete path

- Manually exclude paths

- is an array of strings

### replace

- replace path

```json
        "replace":[
         {"path":"/root","repalce":"./template"},
         {"path":"/temp*","repalce":"./tem"}
        ],
```

### proxy

- Set up the proxy, and j load balancing, the structure is an array of objects

- Each member includes three ingredients
  
  1. The model mode can only be selected from RANDOM, HASH, POLLING, POLLRAN
     
     > random is random by weight
     
     hash is assigned by hash
     
     > polling by weight
     > 
     > pollran polling plus random
  
  2. path is the forwarding path, supports * matching
  
  3. host is an array of servers, the format is ip:port
  
  4. weight is the weight array, the number should be the same as the host, otherwise an error will occur

```json
    //set agent array
     "reverse proxy":[
     {"model":"RANDOM","path":"/try*","host":["127.0.0.1:5201"],"weight":[1]}
     ],
```

### optional

#### memory

- int type, no setting defaults to 1

- The function is to set the initial send buffer size, the unit is M

#### thread num

- Specify the number of threads, int type, the default is 5

- Only works in THREAD mode 
