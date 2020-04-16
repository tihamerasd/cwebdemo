#!/bin/sh

rm ./password.txt
echo -n $1 |shasum -a 512 |cut -d ' ' -f 1 > password.txt
echo "Check password.txt file."
