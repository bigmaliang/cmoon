#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

useage()
{
   echo "useage: $0 DIRS"
   echo "example: $0 foo bar"
   exit -1
}


if [ $# -lt 1 ]; then
    useage
fi

TODAY=`date +%Y%m%d`
PWD=`pwd`
DIRNAME=$(basename "$PWD")

DIRS=$*
for dir in $DIRS
do
    make -C $dir clean
done

tar zcvf ${DIRNAME}_${TODAY}.tar.gz ${DIRS}
