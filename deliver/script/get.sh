#!/bin/bash

num=$1
logfile=/tmp/$num.log
sessid=`wget -q -O - "http://0.push.mangoq.com:6969/2/?[{\"cmd\": \"CONNECT\", \"chl\": 1, \"params\": {\"uin\": \"$1\"}}]" 2>/dev/null |awk -F 'sessid":"|"}},{"time"' '{print $2}'`
echo ${sessid} >> ${logfile}
sleep 3
while true; do
    wget -q -O - "http://0.push.mangoq.com:6969/2/?[{\"cmd\": \"CHECK\", \"chl\": 1, \"sessid\": \"$sessid\"}] " 2>&1 >> ${logfile} & sleep 18;
done
