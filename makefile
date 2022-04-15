#***********************************************
# Author: chenxuan-1607772321@qq.com
# change time:2022-04-15 19:29:51
# description: makefile of project
#**********************************************/
all:main

cc=g++
obj=main
obj_source=./cpp/main.cpp
pro_source=./hpp/cppweb.h
source=./cpp/main.cpp ./hpp/cppweb.h ./hpp/proxy.h ./hpp/config.h ./hpp/route.h
install_dir=/usr/local/include/cppweb
link=-lpthread
link_ssl=-lpthread -lssl -lcrypto
macro=-D CPPWEB_OPENSSL

main: $(source)
	$(cc) -O2 $(obj_source) -o $(obj) $(link) 
clean:
	rm -f *.o 
sql: sql.o
	ar rcs ./lib/libsql.a $^
sql.o: ./cpp/sql.cpp
	$(cc) -c $^ -o $@
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
