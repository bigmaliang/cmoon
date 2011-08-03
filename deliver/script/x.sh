#!/bin/bash

num=$1
logfile=/tmp/$num.log
sessid=`wget -q -O - "http://0.push.mangoq.com:6969/2/?[{\"cmd\": \"CONNECT\", \"chl\": 1, \"params\": {\"uin\": \"17\"}}]" 2>/dev/null |awk -F 'sessid":"|"}},{"time"' '{print $2}'`

     while true; do 
        wget -q -O - "http://0.push.mangoq.com:6969/2/?[{\"cmd\": \"MGQ_SEND\", \"chl\": 1, \"sessid\": \"$sessid\", \"params\": {\"uin\": \"$num\", \"msg\": \"xxx\"}}]" 2>&1 >> ${logfile}
        sleep 1;
    done
