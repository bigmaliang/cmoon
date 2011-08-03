#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

useage()
{
   echo "useage: $0 -m METHOD DIRS"
   echo "example: $0 -m clean foo bar"
   exit -1
}


if [ $# -lt 1 ]; then
    useage
fi

# process parameter
while getopts ':m:' OPT; do
    case $OPT in
        m)
            ACTION="$OPTARG";;
        ?)
            useage
    esac
done
shift $(($OPTIND - 1))

DIRS=$*
for dir in $DIRS
do
    echo "######### $ACTION $dir #########"
    make -C $dir $ACTION
    if [ $? != 0 ]; then
        echo "#########!!!! MAKE $dir FAILURE !!!!#########"
        exit $?
    fi
done
