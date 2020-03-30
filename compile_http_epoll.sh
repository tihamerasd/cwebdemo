#!/bin/sh

#cc="clang"
#cc="gcc"
cc="musl-clang"

rm ./http_epoll
#http kqueue, nonblocking-socket
$cc ./backend/html_templater/flate.c -c -I.
ar -r libflate.a flate.o
$cc ./backend/http_parser/http_parser.c -c -I.
cc -std=gnu11 -pedantic -o http_epoll http_epoll.c ./http_parser.o ./backend/sql/sqlite3.c ./libflate.a\
 ./backend/webapplication_firewall/simple_waf.c ./backend/sql/sqlthings.c ./backend/dynamic_string/sds.c ./backend/keyvalue.c \
 ./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -O3 -L/usr/local/lib -lz -lrt -lpthread -pthread -static -ldl

#valgrind --leak-check=yes -s ./http_epoll
#./http_epoll

