#!/bin/sh

#WARNING
echo "WARNING! ARM-Mbedtls dependency!"

#cc="clang"
cc="gcc"

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
	-lsqlite3 \
	-lmbedtls \
	-lmbedx509 \
	-lmbedcrypto -lcrypto \
	-O3

valgrind --leak-check=full -s ./mbedtls_server
#./mbedtls_server
