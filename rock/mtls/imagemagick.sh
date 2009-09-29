#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

if [ $# -lt 2 ]; then
	echo "useage: $0 DOMAIN PIC_FILENAME"
	echo "example: $0 csc c874a89d1ebd57ea622c1d1e7df815de.jpg"
	exit 1
fi

IMG_ROOT="/data/pic/"
DOMAIN="/"$1"/"
FILENAME=$2
FILEORI=$IMG_ROOT$DOMAIN"ori/"$FILENAME
IMGS="24x24 48x48 120x120 250x250 800X600 1024X768 1440x900"

for IMG in $IMGS; do
    mkdir -p $IMG_ROOT$DOMAIN$IMG
    convert -sample $IMG $FILEORI $IMG_ROOT$DOMAIN$IMG"/"$FILENAME
done

exit 0

