#!/bin/bash
echo 'welcome to easy server';
echo 'choose port to bound';
read port;
echo 'choose index html';
read index;
echo 'choose if run in background(true or false)';
read back;
echo 'choose if use guard(y or n)';
read if_guard;
make
make clean

sed -i  's!\("port":\).*!\1'"${port}"',!g' config.json
sed -i  's!\("background":\).*!\1'"${back}"',!g' config.json
sed -i  's!\("guard":\).*!\1'"${if_guard}"',!g' config.json
sed -i  's!\("default file":"\).*!\1'"${index}"'",!g' config.json

./main2.0 
# if [ $if_guard == 'y' -a $version == '1' ]
# then 
# 	./guard main 
# fi
# if [ $if_guard == 'y' -a $version == '2' ]
# then
# 	./guard main2.0
# fi
echo 'every thing is ok,just enjoy it'
