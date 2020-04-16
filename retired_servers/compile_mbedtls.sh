#!/bin/sh

#WARNING
echo "WARNING! ARM-Mbedtls dependency!"

cc="clang"
#cc="gcc"
#cc="musl-clang"

rm ./mbedtls_server
#https epoll, threading, nonblocking-socket
$cc ./backend/html_templater/flate.c -c -I.
ar -r libflate.a flate.o
$cc ./backend/http_parser/http_parser.c -c -I.
cc 	-std=gnu11 \
	-pedantic \
	-Wall \
	\
	-o mbedtls_server https_fork.c \
	\
	./http_parser.o \
	./libflate.a\
	./backend/webapplication_firewall/simple_waf.c \
	./backend/sql/sqlthings.c \
	./backend/sql/sqlite3.c \
	./backend/dynamic_string/sds.c \
	./backend/keyvalue.c \
	./backend/responser.c \
	./backend/requester.c \
	\
	-I/usr/local/include \
	-L/usr/local/lib \
	\
	-Os \
	-pthread \
	-lm \
	-lz \
	-lrt \
	-lmbedtls \
	-lmbedx509 \
	-lmbedcrypto \
	-O3 \
	-static \
	-ldl

#valgrind --leak-check=full -s ./mbedtls_server
#./mbedtls_server
