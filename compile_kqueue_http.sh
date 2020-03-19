#!/bin/sh

#WARNING
echo "WARNING! Hardcoded path in linking to sslpath. Please install or compile wolfssl and change path!"

cc="clang"
#cc="gcc"

rm ./kqueue_http
#http kqueue, nonblocking-socket
$cc ./backend/html_templater/flate.c -c -I.
ar -r libflate.a flate.o
$cc ./backend/http_parser/http_parser.c -c -I.
cc -std=gnu11 -pedantic -o kqueue_http kqueue.c ./http_parser.o ./libflate.a\
 ./backend/webapplication_firewall/simple_waf.c ./backend/sql/sqlthings.c ./backend/dynamic_string/sds.c ./backend/keyvalue.c \
 ./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -O3 -pthread -L/usr/local/lib -lsqlite3 -lcrypto -lz

#valgrind --leak-check=yes -s ./https_server
#./https_server
