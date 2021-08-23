@echo off
echo please make sure ico is ico.ico
echo please input the name of cpp:
set /p cpp= 
echo please input the name of exe:
set /p exe=
echo please input link:
set /p link=
windres -i ico.rc -o ico.o
g++ %cpp% ico.o -o %exe% -O2 %link%