#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

useage()
{
   echo "useage: $0 -p JSMIN_PATH JSFILES"
   echo "example: $0 -p ../maketool/jsmin foo.js bar.js"
   exit -1
}


if [ $# -lt 2 ]; then
    useage
fi

# process parameter
while getopts 'p:' OPT; do
    case $OPT in
        p)
            JSMIN="$OPTARG";;
        ?)
            useage
    esac
done
shift $(($OPTIND - 1))

JSFILES=$*

for file in $JSFILES
do
    ${JSMIN} <$file
done
