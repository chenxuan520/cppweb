# Makefile by chenxuan in 2021 11 28
all:main.exe main2.0.exe libserver.a libhttp.a libthreadpool.a
main2.0.exe:main2.0.o server.o http.o route.o
	g++ main2.0.o server.o http.o -o main2.0.exe -lwsock32 -lpthread
main2.0.o:./cpp/main2.0.cpp  
	g++ -c ./cpp/main2.0.cpp  -o main2.0.o
main.exe:main.o server.o http.o dealask.o
	g++ main.o server.o http.o dealask.o -o main.exe -lwsock32 -lpthread
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
libserver.a:server.o
	ar rcs ./lib/libserver.a server.o
libhttp.a:http.o
	ar rcs ./lib/libhttp.a http.o
libthreadpool.a:thread.o
	ar rcs ./lib/libthreadpool.a thread.o
clean:
	rm -f *.o 