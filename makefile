all:main init guard main2.0 libserver.a libhttp.a

init:init.o
	g++ init.o -o init
init.o:./cpp/init.cpp
	g++ -c ./cpp/init.cpp -o init.o
guard:guard.o
	g++ guard.o -o guard
guard.o:
	g++ -c ./cpp/guardHttp.cpp -o guard.o
main2.0:main2.0.o server.o http.o dealask.o
	g++ main2.0.o server.o http.o dealask.o -o main2.0 
main2.0.o:./cpp/main2.0.cpp
	g++ -c ./cpp/main2.0.cpp -o main2.0.o
libserver.a:
	ar rcs ./lib/libserver.a server.o
libhttp.a:
	ar rcs ./lib/libhttp.a http.o
main:main.o server.o http.o dealask.o
	g++ main.o server.o http.o dealask.o -o main
main.o:./cpp/main.cpp 
	g++ -c ./cpp/main.cpp -o main.o
server.o:./cpp/server.cpp
	g++ -c ./cpp/server.cpp -o server.o
http.o:./cpp/http.cpp
	g++ -c ./cpp/http.cpp -o http.o
dealask.o:./cpp/dealask.cpp
	g++ -c ./cpp/dealask.cpp -o dealask.o
clean:
	rm -f *.o 
install:
	cp ./lib/*.h /usr/local/include/
	cp ./lib/*.a /usr/local/lib/
