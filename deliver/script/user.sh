if [[ $# -lt 3 ]] ;then echo "use: $0 xx.sh addnum num*100"&&exit
 fi;
m=0
add=$2
nn=$3
for ((i=0;i<$nn;i++))
 do
        for ((j=1 ;j<=100;j++))
        #do m=$((160000+i*100+j));
        do m=$((add+i*100+j));
    sh  ./$1 $m & 
echo $m >num
        done
    sleep 4;
    #ssh 192.168.8.216 "/root/shell/status.sh" >status/$m.status
done
