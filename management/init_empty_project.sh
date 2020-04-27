#!/bin/bash

rm -r ./../build
rm -r ./../dev/
mkdir ./../dev
mkdir ./../dev/frontend
mkdir ./../dev/db
mkdir ./../dev/routing
echo ""> ./../dev/db/sql_queries.c
echo ""> ./../dev/routing/routes.c
echo "WARNING! Empty routes.c, please define the controllercall() function there."
echo "Please read documentation for more details."
echo "Remove functions from sql_queries.h manually..."
echo "Remove functions from routes.h manually, but recover the controllercall..."
wget https://raw.githubusercontent.com/tihamerasd/cwebdemo/master/dev/config.h -o ./../dev/config.h
echo "WARNING! Config.h recovered, please edit as your needs."
