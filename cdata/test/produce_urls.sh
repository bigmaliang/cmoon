#!/bin/sh

GETFILE="urls/get.urls"
SETFILE="urls/set.urls"
INCFILE="urls/inc.urls"

OUTGET="urls/get.out"
OUTSET="urls/set.out"
OUTINC="urls/inc.out"
OUTGET_SET="urls/get.out.set"
OUTGET_INC="urls/get.out.inc"

DOMAINS="n_user_space n_photo n_photo_album n_video n_video_d n_video_album n_blog"

echo -n > ${GETFILE}
for op in ${DOMAINS}; do
	for key in `seq 300000`; do
		echo "http://rtds.dsc.hunantv.com/cgi-bin/cds.cgi?key=$key&op=$op" >> ${GETFILE}
	done
done

echo -n > ${SETFILE}
for op in ${DOMAINS}; do
	for key in `seq 300000`; do
		echo "http://rtds.dsc.hunantv.com/cgi-bin/cds.cgi?key=$key&op=$op&query=post&val=$key" >> ${SETFILE}
	done
done

echo -n > ${INCFILE}
for op in ${DOMAINS}; do
	for key in `seq 300000`; do
		echo "http://rtds.dsc.hunantv.com/cgi-bin/cds.cgi?key=$key&op=$op&query=put" >> ${INCFILE}
	done
done
for op in ${DOMAINS}; do
	for key in `seq 300000`; do
		echo "http://rtds.dsc.hunantv.com/cgi-bin/cds.cgi?key=$key&op=$op&query=put&inc=$key" >> ${INCFILE}
	done
done

function do_test()
{
	infile=$1
	outfile=$2
	tms=`date +%s`
	echo "==================test ${outfile}...=================="
	echo $tms
	echo -n > ${outfile}
	wget -q --no-cache -O ${outfile} -i ${infile}
	tme=`date +%s`
	echo $tme
	let "diff = $tme - $tms"
	echo "use time(seconds) $diff $tms-$tme" >> ${outfile}
	echo "DONE $diff"
}

do_test $GETFILE $OUTGET
do_test $SETFILE $OUTSET
do_test $GETFILE $OUTGET_SET
do_test $INCFILE $OUTINC
do_test $GETFILE $OUTGET_INC
