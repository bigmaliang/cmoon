#!/bin/sh
# $fkq_mod_bat.sh open/close [uin.txt]

TODAY=`date +%F`
TMPFILE="/usr/local/ape/mtools/fkq_open_$TODAY.txt"

if [ $# -gt 1 ]; then
	TMPFILE=$2
else
	echo -n > $TMPFILE
	echo "SELECT userid FROM user_info WHERE bonus>1000 AND fkq_level != 1;" | /usr/local/mysql/bin/mysql -h 192.168.1.23 -u sq_mysq1_count -psq1_m^sq1_ssap -P3063 -Dhome -N > $TMPFILE
fi

USERS=`cat $TMPFILE`

if [ "$1" = "open" ]; then
    for user in $USERS; do
        /usr/local/bin/uic 1 3002 $user -1 9 -1 1
    done
elif [ "$1" = "close" ]; then
    for user in $USERS; do
        /usr/local/bin/uic 1 3002 $user -1 9 -1 0
    done
else
    echo "unknown operation"
fi
