#!/bin/sh

#WARNING
echo "WARNING! Hardcoded path in linking to sslpath. Please install or compile wolfssl and change path!"

#cc="clang"
cc="gcc"

#https epoll, threading, nonblocking-socket
$cc ./backend/html_templater/flate.c -c -I.
ar -r libflate.a flate.o
$cc ./backend/http_parser/http_parser.c -c -I.
cc -o https_server server-tls-epoll-threaded.c ./http_parser.o ./libflate.a ./backend/dynamic_string/sds.c ./backend/keyvalue.c ./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -I/home/tihi/cweb/wolfssl/wolfssl -Os -pthread -L/usr/local/lib -L/home/tihi/cweb/wolfssl/wolfssl -lm -lwolfssl

#valgrind --leak-check=yes -s ./https_server
#./https_server
