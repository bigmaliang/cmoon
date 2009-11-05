#!/bin/sh

if [ $# -lt 3 ]; then
	echo "useage: $0 process_absolute_name need_count pre_exec exec_cmd"
	echo "example: $0 /data/www/rtds/cgi-bin/cds.cgi 4 'cd /data/www/rtds/cgi-bin/' ./cds.cgi"
	exit -1
fi

TIMENOW=`date +"%F %T"`

FILENAME=$1
NEEDCOUNT=$2
PREEXEC=$3
EXECCMD=$4
PROGNAME=$(basename "$FILENAME")

RUNCOUNT=$(ps fax | grep -v grep | grep -v $0 | grep $PROGNAME | wc -l)
#RUNCOUNT=$[$(ps fax | grep $PROGNAME | wc -l)-2]

if [ $RUNCOUNT -lt $NEEDCOUNT ]; then
	#killall $PROGNAME
    $PREEXEC
	if $EXECCMD; then
		echo "$TIMENOW reexec success!"
	else
		echo "$TIMENOW reexec $EXECCMD return $ret"
	fi
	exit 1
else
	echo "$TIMENOW $PROGNAME still running"
	exit 0
fi
