#!/bin/sh

USERS="
3		 
6		 
775393 
1855123
1671006
778444 
1916312
2046576
1864902
1593207
1751745
"

if [ "$1" = "open" ]; then
	for user in $USERS; do
		/usr/local/bin/uic 1 3002 $user -1 9 -1 1
	done
elif [ "$1" = "close" ]; then
	for user in $USERS; do
		/usr/local/bin/uic 1 3002 $user -1 9 -1 0
	done
else
	echo "unknown operation"
fi
