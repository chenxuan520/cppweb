## Purpose of this article

Install the cppweb framework, which is more convenient to use

## windows

> Personal use

1. Include cppweb.h in the hpp folder to include the header file directly.
2. Add -lpthread -lwsock32 when linking
3. The scope is cppweb

> global use

1. Put hpp/cppweb.h into the compiler header file and you can use it

## Linux

> Personal use

1. git clone the repository

2. Including cppweb.h in the hpp folder can directly include the header file.

3. Add -lpthread when linking

4. The scope is cppweb

> global use

1. git clone repository

2. make install

3. Add -lpthread when linking

4. using namespace cppweb; (or not)

5. make update

6. Remove make uninstall
