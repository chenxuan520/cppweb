#!/bin/bash
echo 'welcome to easy server';
echo 'please choose version 1 or 2';
read version;
echo 'choose port to bound';
read port;
echo 'choose index html';
read index;
echo 'choose input memory';
read memory;
echo 'choose if run in background(true or false)';
read back;
echo 'choose if use mysql(y or n)';
read if_sql;
echo 'choose if use guard(y or n)';
read if_guard;
if [ $if_sql == 'y' ]
then
	make sql
fi
make
make clean

sed -i  's!\("version":\).*!\1'"${version}"',!g' config.json
sed -i  's!\("port":\).*!\1'"${port}"',!g' config.json
sed -i  's!\("memory":\).*!\1'"${memory}"',!g' config.json
sed -i  's!\("background":\).*!\1'"${back}"',!g' config.json
sed -i  's!\("default file":"\).*!\1'"${index}"'",!g' config.json


if [ $version == '1' ]
then
	./main $port $index $memory $back
elif [ $version == '2' ]
then
	./main2.0 $port $index $memory $back
else
	echo 'setting wrong'
fi
if [ $if_guard == 'y' -a $version == '1' ]
then 
	./guard main 
fi
if [ $if_guard == 'y' -a $version == '2' ]
then
	./guard main2.0
fi
echo 'every thing is ok,just enjoy it'
