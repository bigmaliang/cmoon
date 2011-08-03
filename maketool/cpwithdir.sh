#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

useage()
{
   echo "useage: $0 -p DST_PATH FILES"
   echo "example: $0 -p ../js member/foo.js system/bar.js"
   exit -1
}


if [ $# -lt 1 ]; then
    useage
fi

# process parameter
while getopts ':p:' OPT; do
    case $OPT in
        p)
            PATH_DST="$OPTARG";;
        ?)
            useage
    esac
done
shift $(($OPTIND - 1))

FILES=$*

for file in $FILES
do
    DIR=$(dirname "$file")
    F=$(basename "$file")
    mkdir -p $PATH_DST/$DIR
    cp -f $file $PATH_DST/$DIR
done
