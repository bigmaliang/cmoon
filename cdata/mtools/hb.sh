#!/bin/sh

# rpc 数据心跳检测报警程序, 需配合 crontab 定时拉起使用
# PATH/hb.sh, 使用前请修改ip, 号码, db等参数

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

outfile=/tmp/cds.out
logfile=/tmp/cds.log
rtds="192.168.8.53"
leader="15111231681"

# $1 ip, $2 msg
function rpcError {
	for phonenum in $leader; do
		/usr/local/mysql/bin/mysql -h 192.168.8.90 -u root -pmysqlroot -P3306 -Dmonitordb -e "INSERT INTO pei5_smssend (smsContent) VALUES ('$phonenum|$1 $2');"
	done
}

for ip in $rtds; do
	wget --quiet --timeout=5 --tries=1 --save-headers "http://$ip/cgi-bin/cds.cgi?op=n_photo&key=1" -O $outfile
	#STATUS=`sed -n 1p $outfile`
	grep -q "HTTP/1.1 200 OK" $outfile
	if [ "$?" = "0" ]; then
		grep -q "errmsg" $outfile
		if [ "$?" != "0" ]; then
			echo $ip "is fine" >> $logfile
		else
			errmsg=`grep errmsg $outfile`
			rpcError $ip "$errmsg"
		fi
	else
		errmsg=`grep "HTTP/1.1" $outfile`
		rpcError $ip "$errmsg"
	fi
done
