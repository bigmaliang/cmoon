#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

useage()
{
    echo "useage: $0 application file"
    echo "example: $0 home 20090108.html"
    exit -1
}

if [ $# -lt 1 ]; then
    useage
fi

APP=$1
FILE=$2

if [ "$APP" = "home" ]; then
    SRCPATH=/data/www/admin/put/home
    DSTPATH=/data/www/community/public/put
    DSTIP="192.168.8.190"
    USER="root"
    PASS="hunantv1234"
elif [ "$APP" = "active" ]; then
    SRCPATH=/data/www/admin/put/active
    DSTPATH=/data/www/community
    DSTIP="192.168.8.54"
    USER="root"
    PASS="hifly1234"
else
    echo "don't know how to sync $1"
    exit 1
fi

for ip in $DSTIP
do
    /data/www/admin/script/scp_exp.sh $USER $PASS $ip $SRCPATH/$FILE $DSTPATH/$FILEk
done

echo "done"
