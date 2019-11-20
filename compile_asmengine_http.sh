#!/bin/sh

echo "WARNING! Nasm is my assembler, pls install it if u got errors! (apt-get install nasm/pacman -S nasm)"

#cc="clang"
cc="gcc"

rm ./asmengine_server;
nasm -f elf64 server.asm -o server_asm.o;
$cc ./backend/html_templater/flate.c -c -I.
ar -r libflate.a flate.o
$cc -no-pie -fPIC server.c server_asm.o ./backend/requester.c ./libflate.a ./backend/keyvalue.c ./backend/responser.c ./backend/dynamic_string/sds.c -o asmengine_server -Wno-implicit-function-declaration -pthread;

#debug
#valgrind --leak-check=yes -s ./asmengine_server

#product
#./asmengine_server
