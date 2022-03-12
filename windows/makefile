all:main

main:./cpp/main.cpp ./hpp/cppweb.h
	g++ ./cpp/main.cpp -o main -lpthread -lwsock32 -std=c++11
clean:
	rm -f *.o 
sql: sql.o
	ar rcs ./lib/libsql.a sql.o
sql.o: ./cpp/sql.cpp
	g++ -c ./cpp/sql.cpp -o sql.o
ssl:./hpp/cppweb.h ./hpp/config.h ./cpp/main.cpp ./hpp/route.h
	g++ ./cpp/main.cpp -o main -D CPPWEB_OPENSSL -lpthread -lwsock32 -lssl -lcrypto -std=c++11
install:
	mkdir /usr/local/include/cppweb
	cp hpp/cppweb.h /usr/local/include/cppweb
update:
	cp hpp/cppweb.h /usr/local/include/cppweb
uninstall:
	rm -rf /usr/local/include/cppweb
