
all:main

cc=g++
obj_source=./main.cpp
obj=main
link=-lpthread
link_ssl=-lpthread -lssl -lcrypto
.PHONY:main

main: $(source)
	$(cc) -g $(obj_source) -o $(obj) $(link) $(link_ssl)
