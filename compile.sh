#!/bin/sh

#cc="clang"
cc="gcc"
rm ./server;
#gcc -c -o threadpool.o threadpool.c;
nasm -f elf64 server.asm -o server_asm.o;
$cc -no-pie -fPIC server.c server_asm.o requester.c keyvalue.c responser.c sds.c -o server -Wno-implicit-function-declaration -pthread;

#valgrind --leak-check=yes -s ./server
#./server
