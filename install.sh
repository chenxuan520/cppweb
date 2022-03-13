#!/bin/bash
echo 'welcome to easy server';
echo 'choose port to bound';
read port;
echo 'choose index html';
read index;
echo 'choose if run in background(true or false)';
read back;
echo 'choose if use guard(true or false)';
read if_guard;
echo 'choose if use https(true or false)';
echo 'if use you must change certway in config.json'
read https;
if [ $https == 'false' ]
then
	make
else
	make ssl
fi
make clean

sed -i  's!\("port":\).*!\1'"${port}"',!g' config.json
sed -i  's!\("background":\).*!\1'"${back}"',!g' config.json
sed -i  's!\("guard":\).*!\1'"${if_guard}"',!g' config.json
sed -i  's!\("default file":"\).*!\1'"${index}"'",!g' config.json

./main 
echo 'every thing is ok,just enjoy it'
