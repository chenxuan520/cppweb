all:main init guard

init:init.o
	g++ init.o -o init
init.o:./cpp/init.cpp
	g++ -c ./cpp/init.cpp -o init.o
guard:guard.o
	g++ guard.o -o guard
guard.o:
	g++ -c ./cpp/guardHttp.cpp -o guard.o
main:main.o sql.o server.o http.o dealask.o
	g++ main.o sql.o server.o http.o dealask.o -o main -lmysqlclient
main.o:./cpp/main.cpp 
	g++ -c ./cpp/main.cpp -o main.o
sql.o:./cpp/sql.cpp
	g++ -c ./cpp/sql.cpp -o sql.o
server.o:./cpp/server.cpp
	g++ -c ./cpp/server.cpp -o server.o
http.o:./cpp/http.cpp
	g++ -c ./cpp/http.cpp -o http.o
dealask.o:./cpp/dealask.cpp
	g++ -c ./cpp/dealask.cpp -o dealask.o
clean:
	rm -f *.o 
