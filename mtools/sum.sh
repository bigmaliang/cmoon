#!/bin/bash

seq=$1
sum=0

for i in `seq $seq`; do
    if [ $((i%2)) == 0 ]; then
        echo -n
    elif [ $((i%3)) == 0 ]; then
        echo -n
    else
        echo $i
        sum=$((sum+i))
    fi
done

echo "sum" $sum
