
all:main

cc=g++
obj_source=./main.cpp
obj=main
link=-lpthread
link_ssl=-lpthread -lssl -lcrypto
.PHONY:main

main: $(source)
	$(cc) -O2 $(obj_source) -o $(obj) $(link) $(link_ssl)
