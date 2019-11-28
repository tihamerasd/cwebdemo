#!/bin/sh

#WARNING
echo "WARNING! Hardcoded path in linking to sslpath. Please install or compile wolfssl and change path!"

cc="clang"
#cc="gcc"

rm ./wss_socket
#https epoll, threading, nonblocking-socket
$cc ./backend/html_templater/flate.c -c -I.
ar -r libflate.a flate.o
$cc ./backend/http_parser/http_parser.c -c -I.
cc -std=gnu11 -pedantic -o wss_socket server-tls-epoll-perf.c ./http_parser.o ./libflate.a ./backend/webapplication_firewall/yarawaf.c ./backend/dynamic_string/sds.c ./backend/keyvalue.c ./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -I/home/tihi/cweb/wolfssl/wolfssl -Os -pthread -L/usr/local/lib -L/home/tihi/cweb/wolfssl/wolfssl -lm -lwolfssl -lssl -lcrypto -lyara

#valgrind --leak-check=yes -s ./wss_socket
./wss_socket

