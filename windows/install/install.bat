@echo off
mkdir ..\x86_64-w64-mingw32\include\cppweb
copy .\*.h ..\x86_64-w64-mingw32\include\cppweb
copy .\*.a ..\x86_64-w64-mingw32\lib
pause