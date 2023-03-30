#!/bin/bash
git clone https://gitclone.com/github.com/redis/hiredis.git
cd hiredis
make 
make install
/sbin/ldconfig
apt-get install libmysqlclient-dev libmysql++-dev
cd ..
make sql
echo 'install redis and mysql ok'
