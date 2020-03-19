#!/bin/bash

rm ./password.txt
echo -n $1 |sha512sum |cut -d ' ' -f 1 > password.txt
echo "Check password.txt file."
