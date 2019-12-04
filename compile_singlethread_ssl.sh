#!/bin/sh

#WARNING
echo "WARNING! Hardcoded path in linking to sslpath. Please install or compile wolfssl and change path!"

#cc="clang"
cc="gcc"

rm ./singlethread_https_server
#https epoll, threading, nonblocking-socket
$cc ./backend/html_templater/flate.c -c -I.
ar -r libflate.a flate.o
$cc ./backend/http_parser/http_parser.c -c -I.
cc -std=gnu11 -pedantic -o singlethread_https_server https_single_thread.c ./http_parser.o ./libflate.a ./backend/webapplication_firewall/yarawaf.c ./backend/dynamic_string/sds.c ./backend/keyvalue.c ./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -I/home/tihi/cweb/wolfssl/wolfssl -Os -pthread -L/usr/local/lib -L/home/tihi/cweb/wolfssl/wolfssl -lm -lwolfssl -lyara

#valgrind --leak-check=yes -s ./https_server
#./https_server
