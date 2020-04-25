#!/bin/bash

rm -r ./../build
startpath=`pwd`
mkdir /tmp/tmpcweb
cd /tmp/tmpcweb
git clone https://github.com/tihamerasd/cwebdemo.git
cd cwebdemo
cp -r ./server $startpath
cp -r ./management $startpath
cd $startpath
