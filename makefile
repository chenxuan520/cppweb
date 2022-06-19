#***********************************************
# Author: chenxuan-1607772321@qq.com
# change time:2022-04-15 19:29:51
# description: makefile of project
#**********************************************/
all:main

cc=g++
obj=main
obj_source=./src/cpp/main.cpp
pro_source=./src/hpp/cppweb.h
source=./src/cpp/main.cpp ./src/hpp/cppweb.h ./src/hpp/proxy.h ./src/hpp/config.h ./src/hpp/route.h ./src/hpp/argc.h
link=-lpthread
link_ssl=-lpthread -lssl -lcrypto
macro=-D CPPWEB_OPENSSL
install_dir=/usr/local/include/cppweb

main: $(source)
	$(cc) -O2 $(obj_source) -o $(obj) $(link) 
clean:
	rm -f *.o 
ssl: $(source)
	$(cc) -O2 $(obj_source) -o $(obj) $(macro) $(link_ssl)
debug:
	$(cc) -g $(obj_source) -o $(obj) $(link)
install:
	mkdir $(install_dir)
	cp $(pro_source) $(install_dir)
update:
	cp $(pro_source) $(install_dir)
uninstall:
	rm -rf $(install_dir)
