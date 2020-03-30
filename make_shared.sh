#!/bin/sh

#cc="clang"
cc="gcc"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/root/cwebdemo/

rm ./shared_cweb.so
$cc ./backend/html_templater/flate.c -c -I.
ar -r libflate.a flate.o
$cc ./backend/http_parser/http_parser.c -c -I.
ar -r libhttp_parser.a http_parser.o
cc -shared -fPIC -o libshared_cweb.so -I -std=gnu11 -pedantic sharedinterface.c \
 ./backend/webapplication_firewall/simple_waf.c ./backend/sql/sqlthings.c ./backend/sql/sqlite3.c  ./backend/dynamic_string/sds.c ./backend/keyvalue.c \
 ./backend/responser.c ./backend/requester.c -Wall -I/usr/local/include -I/home/tihi/cweb/wolfssl/wolfssl \
 -Os -pthread -L/usr/local/lib -lm -lsqlite3 -lz
go build goserver.go

./goserver
