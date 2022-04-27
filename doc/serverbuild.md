## Goal of this article

- Use cppweb to build a simple http server and https server

## Script installation tutorial

> linux available

1. git clone https://gitee.com/chenxuan520/server-for-static-web.git

2. ./install.sh

3. Fill in the port and other information

4. Server running
   
   ##### Fill in the information

5. Fill in the bound port (eg 5200)

6. Fill in the default html file name (index.html) of the web page

7. Fill in whether to run in the background (true or false)

8. Fill in whether to use daemon (true or false)

## Modify the configuration method (recommended)

> Both windows and linux are available

1. git clone https://gitee.com/chenxuan520/server-for-static-web.git

2. make (make -f makefile.win under windows)
   
   > If you have https certificate, you can also make ssl

3. Modify config.json,[introduction](./configjson.md)

4. ./main

## Information Description

- If not running in the background, the shell will end together

- The daemon process can ensure fast restart after downtime

## Change setting

- Modify config.json directly

- ./main --reload

## stop the server

- ./main --stop

## Specify the configuration file

- ./main --config=(file name)

## View help

- ./main --help

## Build https service

### illustrate

1. Used to modify the configuration method, the script method is invalid

2. You need to install openssl in advance
   
   apt install openssl openssl-dev

### 1. Get the certificate

#### openssl self-signed (for testing)

```shell
openssl genrsa -des3 -out privkey.pem 2048
openssl req -new -key privkey.pem -out cert.csr
openssl req -new -x509 -key privkey.pem -out cacert.pem -days 1095
```

#### Get ssl certificate using domain name

1. The acquisition path is the domain name purchase place or [freessl](https://freessl.cn/)

2. Download the certificate for nginx, including a pem and a key

3. Convert key to pem

```shell
openssl rsa -in (key name).key -out (new name).pem
```

### 2. Modify config.h

- Change the values ​​of "cert path" and "key path" to the certificate path

- if there is no password comment the line "cert password"

### 3. Compile and run

1. **make ssl**

2. ./main
