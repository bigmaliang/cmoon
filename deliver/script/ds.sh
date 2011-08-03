#!/bin/bash

num=$1
logfile=/tmp/$num.log
rr=$((num%2))
sessid=`wget -q -O - "http://0.push.mangoq.com:6969/2/?[{\"cmd\": \"CONNECT\", \"chl\": 1, \"params\": {\"uin\": \"$1\"}}]" 2>/dev/null |awk -F 'sessid":"|"}},{"time"' '{print $2}'`

if [[ $rr == 0 ]]; then
     while true; do 
        wget -q -O - "http://0.push.mangoq.com:6969/2/?[{\"cmd\": \"MGQ_SEND\", \"chl\": 1, \"sessid\": \"$sessid\", \"params\": {\"uin\": \"$((num-1))\", \"msg\": \"xxx\"}}]" 2>&1 >> ${logfile}
        sleep 1;
    done
else
     while true ; do 
        wget -q -O - "http://0.push.mangoq.com:6969/2/?[{\"cmd\": \"CHECK\", \"chl\": 1, \"sessid\": \"$sessid\"}] " 2>&1 >> ${logfile} & sleep 18;
    done
fi
