#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

TIMENOW=`date +"%F %T"`
outfile=/tmp/push.out
logfile=/tmp/push.log
server="0.push.hunantv.com"
leader="15111231681"

# $1 ip, $2 msg
function rpcError {
	for phonenum in $leader; do
		/usr/local/mysql/bin/mysql -h 192.168.8.90 -u root -pmysqlroot -P3306 -Dmonitordb -e "INSERT INTO pei5_smssend (smsContent) VALUES ('$phonenum|$1 $2');"
	done
}

for ip in $server; do
	wget --quiet --timeout=5 --tries=1 --no-cache --save-headers "http://$ip/?CONNECT&19820222&1&123" -O $outfile
	
	if grep -q "HTTP/.* 200 OK" $outfile && grep -q "CHANNEL" $outfile; then
		if grep -q "ERR" $outfile; then
			errmsg=`grep ERR $outfile`
			rpcError $ip "$errmsg"
		else
			echo $TIMENOW $ip "is fine" >> $logfile
		fi
	else
		errmsg=`grep "HTTP/.*" $outfile`
		rpcError $ip "$errmsg"
	fi
done
