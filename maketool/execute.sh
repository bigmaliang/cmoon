#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

files=$*

for file in $files
do
    ./$file
done

return 0
