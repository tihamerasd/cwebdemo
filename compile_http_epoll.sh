#!/bin/sh

cc="clang"
#cc="gcc"
#cc="musl-clang"

rm -r ./build
mkdir build

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tihi/cweb/build

$cc -o ./build/flate.o ./server/backend/html_templater/flate.c -c -I.
ar -r ./build/libflate.a ./build/flate.o
$cc -o ./build/http_parser.o ./server/backend/http_parser/http_parser.c -c -I.
cc -std=gnu11 -pedantic \
	-o ./build/http_epoll ./server/socket_handlers/http_epoll.c \
 ./build/http_parser.o \
 ./build/flate.o \
 ./server/backend/sql/sqlite3.c \
 ./server/backend/webapplication_firewall/simple_waf.c \
 ./server/backend/sql/sqlthings.c \
 ./server/backend/dynamic_string/sds.c \
 ./server/backend/keyvalue.c \
 ./server/backend/responser.c \
 ./server/backend/requester.c \
 ./server/backend/controller.c \
 -Wall -I/usr/local/include -O3 -L/usr/local/lib -lz -lrt -lpthread -pthread -lcrypto -ldl

valgrind --leak-check=yes -s ./build/http_epoll
#./build/http_epoll

