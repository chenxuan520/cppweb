#!/bin/bash
echo 'welcome to easy server';
echo 'choose port to bound';
read port;
echo 'choose index html';
read index;
echo 'choose if run in background';
read back;
echo 'choose input memory';
read memory;
echo 'choose if use mysql?';
read if_sql;
if [ $if_sql == 'n' ]
then
    mv ./makefile makeBeifen;
    mv ./makefileNoSql makefile
fi
make
make clean
echo $port $index $memory $back > my.ini
echo 'every thing is ok'
./main $port $index $memory $back
