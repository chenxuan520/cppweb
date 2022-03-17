## Goal of this article

- Use cppweb to build a simple http server

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

## Modify the configuration method

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

- Wait about half a minute

## stop the server

- ./main --stop
