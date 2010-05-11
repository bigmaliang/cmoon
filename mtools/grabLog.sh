#!/bin/sh
LC_ALL=C
PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

useage()
{
        echo "useage: $0 minutes logfile"
        echo "example: $0 5 home-acess.log"
        exit -1
}

if [ $# -lt 1 ]; then
        useage
fi

MIN=$1
INFILE=$2

e=UNEXISTBALLLLLLLLL
for (( i = $MIN; i >= 0; i-- )) ; do
    e=`date +%d/%b/%Y:%H:%M -d "-$i min"`'|'$e
done

egrep $e $INFILE


#TIME_CUR="11/May/2010:16:[01-04]"xs
#M_BGN=`date --date="-$MIN minutes" +%M`
#TIME_CUR=`date +%d/%b/%Y:%H:[$M_BGN-%M]`
#TIME_CUR="11/May/2010:16:[47-52]"
#e=
#for (( i = 5; i >= 0; i-- )) ; do
#    e='-e @'`date +\%d/\%b/\%Y:\%H:\%M -d "-$i min"`'@p '$e
#done
#$(sed -n $e $INFILE)
