all:main main2.0

guard.o:./cpp/guardHttp.cpp
	g++ -c ./cpp/guardHttp.cpp -o guard.o
main2.0:main2.0.o ./hpp/cppweb.h
	g++ main2.0.o -o main2.0 -lpthread 
main2.0.o:./cpp/main2.0.cpp 
	g++ -c ./cpp/main2.0.cpp  -o main2.0.o
main:main.o dealask.o 
	g++ main.o dealask.o -o main
main.o:./cpp/main.cpp ./hpp/cppweb.h 
	g++ -c ./cpp/main.cpp -o main.o
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
	cp hpp/cppweb.h /usr/local/include/cppweb
update:
	cp hpp/cppweb.h /usr/local/include/cppweb
uninstall:
	rm -rf /usr/local/include/cppweb
