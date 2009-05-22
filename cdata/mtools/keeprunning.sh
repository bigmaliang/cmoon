#!/bin/sh

if [ $# -lt 3 ]; then
	echo "useage: $0 process_absolute_name need_count exec_cmd"
	echo "example: $0 /data/www/rtds/cgi-bin/cds.cgi 4 '/usr/local/bin/spawn-fcgi -f /data/www/rtds/cgi-bin/cds.cgi -a 192.168.8.53 -p 20010 -F 4'"l
	exit -1
fi

TIMENOW=`date +"%F %T"`

FILENAME=$1
NEEDCOUNT=$2
EXECCMD=$3
PROGNAME=$(basename "$FILENAME")

RUNCOUNT=$[$(ps fax | grep $PROGNAME | wc -l)-2]

if [ $RUNCOUNT -lt $NEEDCOUNT ]; then
	$EXECCMD
	ret=$?
	if [ $ret = "0" ]; then
		echo "$TIMENOW reexec success!"
	else
		echo "$TIMENOW reexec $EXECCMD return $ret"
	fi
	exit 1
else
	echo "$TIMENOW $PROGNAME still running"
	exit 0
fi
