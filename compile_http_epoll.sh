#!/bin/sh

cc="clang"
#cc="gcc"
#cc="musl-clang"

rm -r ./build
mkdir build

#static openssl for static link
#cp /home/tihi/openssl-static/src/openssl-1.1.1f/libcrypto.a /home/tihi/cweb/build/

$cc -O3 -o ./build/flate.o ./server/backend/html_templater/flate.c -c -I.
ar -r ./build/libflate.a ./build/flate.o
$cc -O3 -o ./build/http_parser.o ./server/backend/http_parser/http_parser.c -c -I.
$cc -g -Wl,-z,relro,-z,now \
  -std=gnu11 -pedantic \
 -fstack-protector-strong \
 -o ./build/http_epoll \
 ./server/socket_handlers/http_epoll.c \
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

valgrind --leak-check=yes -s ./build/http_epoll
#./build/http_epoll

