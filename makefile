all:main

main:./cpp/main.cpp ./hpp/cppweb.h ./hpp/config.h ./hpp/route.h
	g++ ./cpp/main.cpp -o main -lpthread 
clean:
	rm -f *.o 
sql: sql.o
	ar rcs ./lib/libsql.a sql.o
sql.o: ./cpp/sql.cpp
	g++ -c ./cpp/sql.cpp -o sql.o
ssl:./hpp/cppweb.h ./hpp/config.h ./cpp/main.cpp ./hpp/route.h
	g++ ./cpp/main.cpp -o main -D CPPWEB_OPENSSL -lpthread -lssl -lcrypto
install:
	mkdir /usr/local/include/cppweb
	cp hpp/cppweb.h /usr/local/include/cppweb
update:
	cp hpp/cppweb.h /usr/local/include/cppweb
uninstall:
	rm -rf /usr/local/include/cppweb
debug:
	g++ -g ./cpp/main.cpp -o main -lpthread
