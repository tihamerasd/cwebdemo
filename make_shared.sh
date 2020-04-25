#!/bin/sh

cc="clang"
#cc="gcc"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tihi/cweb/build

rm -r ./build
mkdir build

$cc -o ./build/flate.o ./server/backend/html_templater/flate.c -c -I. -O3
ar -r ./build/libflate.a ./build/flate.o
$cc -o ./build/http_parser.o ./server/backend/http_parser/http_parser.c -c -I. -O3
ar -r ./build/libhttp_parser.a ./build/http_parser.o
cc -shared -fPIC -o ./build/libshared_cweb.so -I -std=gnu18 -pedantic \
	./server/sharedinterface.c \
	./server/backend/webapplication_firewall/simple_waf.c \
	./server/backend/sql/sqlthings.c \
	./server/backend/sql/sqlite3.c \
	./server/backend/dynamic_string/sds.c ./server/backend/keyvalue.c \
	./server/backend/responser.c \
	./server/backend/requester.c \
	./server/backend/controller.c \
	-Wall  -I/usr/local/include -O3 -pthread -L/usr/local/lib -lm -lsqlite3 -lz

go build -o ./build/goserver ./server/socket_handlers/goserver.go

./build/goserver
