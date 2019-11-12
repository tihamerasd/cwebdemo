#!/bin/sh

rm ./server;
nasm -f elf64 server.asm -o server_asm.o;
gcc -no-pie -fPIC server.c server_asm.o requester.c keyvalue.c responser.c sds.c -o server -Wno-implicit-function-declaration;

./server
