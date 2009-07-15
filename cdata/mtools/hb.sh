#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

outfile=/tmp/cds.out
logfile=/tmp/cds.log
rtds="192.168.8.53"
leader="15111231681"

for ip in $rtds; do
	wget --quiet --timeout=1 --tries=2 --save-headers http://$ip/cgi-bin/cds.cgi -O $outfile
	#STATUS=`sed -n 1p $outfile`
	grep -q "HTTP/1.1 200 OK" $outfile
	STATUS=$?
	if [ "$STATUS" = "0" ]; then
		echo $ip "is fine" >> $logfile
	else
		STATUS=`grep "HTTP/1.1" $outfile`
		for phonenum in $leader; do
			/usr/local/mysql/bin/mysql -h 192.168.8.90 -u root -pmysqlroot -P3306 -Dmonitordb -e "INSERT INTO pei5_smssend (smsContent) VALUES (\"$phonenum|$ip $STATUS\");"
		done
	fi
done