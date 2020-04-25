#!/bin/bash

#clean build
rm -r ./../build

#update libflate
wget https://raw.githubusercontent.com/ac000/libflate/master/flate.c -o ../server/backend/html_templater/flate.c
wget https://raw.githubusercontent.com/ac000/libflate/master/flate.h -o ../server/backend/html_templater/flate.h

#update http parser
wget https://raw.githubusercontent.com/nodejs/http-parser/master/http_parser.h -o ../server/backend/http_parser/http_parser.c
wget https://raw.githubusercontent.com/nodejs/http-parser/master/http_parser.c -o ../server/backend/http_parser/http_parser.h

#update sds strings
wget https://raw.githubusercontent.com/antirez/sds/master/sds.h -o ../server/backend/dynamic_string/sds.h
wget https://raw.githubusercontent.com/antirez/sds/master/sds.c -o ../server/backend/dynamic_string/sds.c
wget https://raw.githubusercontent.com/antirez/sds/master/sdsalloc.h -o ../server/backend/dynamic_string/sdsalloc.h

echo "sqlite-amalgamation update is not implemented, pls do it manually. https://www.sqlite.org/download.html"

./update.sh
