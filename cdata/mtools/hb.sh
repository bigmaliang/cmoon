#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

TIMENOW=`date +"%F %T"`
outfile=/tmp/cds.out
logfile=/tmp/cds.log
rtds="192.168.1.8 192.168.1.9"
leader="13973178767 13574867947 15111231681"

# $1 ip, $2 msg
function rpcError {
    echo $TIMENOW $1 $2 >> $logfile
	for phonenum in $leader; do
		/usr/local/mysql/bin/mysql -h 222.240.139.132 -u root -pmysqlroot -P3306 -Dmonitordb -e "INSERT INTO monitor_smssend (smsSendTo, smsContent) VALUES ('$phonenum', '$1 $2');"
	done
}

for ip in $rtds; do
	wget --quiet --timeout=5 --tries=1 --save-headers "http://$ip/cgi-bin/cds.cgi?op=n_photo&key=1" -O $outfile
	#STATUS=`sed -n 1p $outfile`
	
	if grep -q "HTTP/.* 200 OK" $outfile && grep -q "success" $outfile; then
		if grep -q "errmsg" $outfile; then
			errmsg=`grep errmsg $outfile`
			rpcError $ip "$errmsg"
		else
			echo $TIMENOW $ip "is fine" >> $logfile
		fi
	else
		errmsg=`grep "HTTP/.*" $outfile`
		rpcError $ip "$errmsg"
	fi
done
