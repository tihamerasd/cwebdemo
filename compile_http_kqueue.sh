#!/bin/sh

cc="clang"
#cc="gcc"

rm -r ./build
mkdir build

$cc -fPIC -O3 -o ./build/flate.o ./server/backend/html_templater/flate.c -c -I.
ar -r ./build/libflate.a ./build/flate.o
$cc -fPIC -O3 -o ./build/http_parser.o ./server/backend/http_parser/http_parser.c -c -I.
$cc -g -Wl,-z,relro,-z,now \
 -s -std=gnu11 -pedantic \
 -fstack-protector-strong \
 -o ./build/http_kqueue \
 ./server/socket_handlers/kqueue.c \
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
 ./dev/routing/routes.c \
 ./dev/db/sql_queries.c \
 -Wall -I/usr/local/include -O3 -L./build -pthread -lcrypto -lz -ldl

#valgrind --leak-check=yes -s ./http_kqueue
./build/http_kqueue
