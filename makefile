all:main init guard main2.0 libserver.a libhttp.a libthreadpool.a

init:init.o
	g++ init.o -o init
init.o:./cpp/init.cpp
	g++ -c ./cpp/init.cpp -o init.o
guard:guard.o
	g++ guard.o -o guard
guard.o:./cpp/guardHttp.cpp
	g++ -c ./cpp/guardHttp.cpp -o guard.o
main2.0:main2.0.o ./hpp/cppweb.h
	g++ main2.0.o -o main2.0 -lpthread 
main2.0.o:./cpp/main2.0.cpp 
	g++ -c ./cpp/main2.0.cpp  -o main2.0.o
libserver.a:server.o
	ar rcs ./lib/libserver.a server.o
libhttp.a:http.o
	ar rcs ./lib/libhttp.a http.o
libthreadpool.a:thread.o
	ar rcs ./lib/libthreadpool.a thread.o
main:main.o server.o http.o dealask.o
	g++ main.o server.o http.o dealask.o -o main
main.o:./cpp/main.cpp  
	g++ -c ./cpp/main.cpp -o main.o
server.o:./cpp/server.cpp
	g++ -c ./cpp/server.cpp -o server.o
http.o:./cpp/http.cpp
	g++ -c ./cpp/http.cpp -o http.o
thread.o:./cpp/thread.cpp
	g++ -c ./cpp/thread.cpp -o thread.o
dealask.o:./cpp/dealask.cpp
	g++ -c ./cpp/dealask.cpp -o dealask.o
route.o:./hpp/route.h
	g++ -c ./hpp/route.h -o route.o
clean:
	rm -f *.o 
sql: sql.o
	ar rcs ./lib/libsql.a sql.o
sql.o: ./cpp/sql.cpp
	g++ -c ./cpp/sql.cpp -o sql.o
install:
	mkdir /usr/local/include/cppweb
	cp ./lib/*.h /usr/local/include/cppweb
	cp ./lib/*.a /usr/local/lib/
uninstall:
	rm -rf /usr/local/include/cppweb
	rm /usr/local/lib/libhttp.a /usr/local/lib/libserver.a /usr/local/lib/libthreadpool.a
