#!/bin/sh

if [ $# -lt 6 ]; then
	echo "useage: $0 process_name need_count cond_exec pre_exec exec_cmd post_exec"
	echo 'example: $0 aped 1 "$(ls -al /corefile/ | wc -l) -gt 3" "cd /usr/local/ape/bin" ./aped "mv /corefile/* /oldcore/"'
	exit -1
fi

TIMENOW=`date +"%F %T"`

FILENAME=$1
NEEDCOUNT=$2
CONDEXEC=$3
PREEXEC=$4
EXECCMD=$5
POSTEXEC=$6
PROGNAME=$(basename "$FILENAME")

RUNCOUNT=$(ps fax | grep -v grep | grep -v $0 | grep $PROGNAME | wc -l)
#RUNCOUNT=$[$(ps fax | grep $PROGNAME | wc -l)-2]

if [ $RUNCOUNT -lt $NEEDCOUNT ]; then
	if [ $CONDEXEC ]; then
		killall $PROGNAME
		$PREEXEC
		if $EXECCMD; then
			echo "$TIMENOW reexec success!"
		else
			echo "$TIMENOW reexec $EXECCMD return $?"
		fi
		$POSTEXEC
	else
		echo "$TIMENOW $PROGNAME don't exist, but con_exec return false, wink"
	fi
	exit 1
else
	echo "$TIMENOW $PROGNAME still running"
	exit 0
fi
