#!/bin/bash
echo 'welcome to easy server';
echo 'please choose version 1 or 2';
read version;
echo 'choose port to bound';
read port;
echo 'choose index html';
read index;
echo 'choose if run in background';
read back;
echo 'choose input memory';
read memory;
echo 'choose if use mysql';
read if_sql;
echo 'choose if use guard';
read if_guard;
if [ $if_sql == 'n' ]
then
    mv ./makefile makeBeifen;
    mv ./makefileNoSql makefile
fi
make
make clean
echo $port $index $memory $back > my.ini
if [ $version == '1' ]
then
	./main $port $index $memory $back
elif [ $version == '2' ]
	./main2.0 $port $index $memory $back
else
	echo 'setting wrong'
fi
if [ $if_guard == 'y' ]
then 
	./guard main 
fi
echo 'every thing is ok'
