#!/bin/sh

if [ `ls a.txt` ]; then
    echo "a exist"
else
    echo "a don't exist"
fi
