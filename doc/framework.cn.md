## 本文目的

安装cppweb框架,使用更方便

## windows

>  个人使用

1. 在hpp文件夹下包含cppweb.h可以直接包含该头文件.

2. 在链接时加上-lpthread -lwsock32

3. 作用域为cppweb

> 全局使用

1. 把hpp/cppweb.h放到编译器头文件就可以使用

## Linux

> 个人使用

1. git clone 仓库

2. 在hpp文件夹下包含cppweb.h可以直接包含该头文件.

3. 在链接时加上-lpthread

4. 作用域为cppweb

> 全局使用

1. git clone仓库

2. make install

3. 在链接时加上-lpthread

4. using namespace cppweb;(也可以不用)

5. 更新 make update

6. 删除 make uninstall
