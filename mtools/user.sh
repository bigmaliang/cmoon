#!/bin/bash

m=0
for ((i=0;i<80;i++))
 do
        for ((j=1 ;j<=100;j++))
        #do m=$((160000+i*100+j));
        do m=$((30000+i*100+j));
	sh  ./$1 $m & 
echo $m >num
        done
	sleep 4;
	#ssh 192.168.8.216 "/root/shell/status.sh" >status/$m.status
done
