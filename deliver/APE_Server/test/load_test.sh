#!/bin/sh

CONNECTFILE="urls/connect.urls"
SENDFILE="urls/send.urls"
CHECKFILE="urls/check.urls"

OUTCONNECT="urls/connect.out"
OUTSEND="urls/send.out"
OUTCHECK="urls/check.out"


echo -n > ${CONNECTFILE}
for key in `seq 10000`; do
	echo "http://0.push.hunantv.com/?CONNECT&$key&1&$key" >> ${CONNECTFILE}
done

echo -n > ${SENDFILE}
for key in `seq 10000`; do
	echo "http://0.push.hunantv.com/?TRUSTSEND&$key&{pageclass%3A1,content:%22message$key%22}&$key" >> ${SENDFILE}
done

echo -n > ${CHECKFILE}
for key in `seq 10000`; do
	echo "http://1.push.hunantv.com/?TRUSTCHECK&$key&$key" >> ${CHECKFILE}
done

function do_test()
{
	infile=$1
	outfile=$2
	tms=`date +%s`
	echo "==================test ${outfile}...=================="
	echo $tms
	echo -n > ${outfile}
	wget -q --no-cache --timeout=5 --tries=1 -O ${outfile} -i ${infile}
	tme=`date +%s`
	echo $tme
	let "diff = $tme - $tms"
	echo "use time(seconds) $diff $tms-$tme" >> ${outfile}
	echo "DONE $diff"
}

do_test $CONNECTFILE $OUTCONNECT
do_test $SENDFILE $OUTSEND
do_test $CHECKFILE $OUTCHECK
