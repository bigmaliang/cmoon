#!/bin/sh

TODAY=`date +%F`
DATADIR=/data/qdbm/
MTLDIR=/data/www/rtds/mtools/

killall nmdb

cp -f ${DATADIR}user  ${MTLDIR}dbs/user_${TODAY}  &&
/usr/local/bin/nmdb -d /data/qdbm/user -t 26010 -u 26010 -c 30720 -o /var/log/nmdb.user &&
${MTLDIR}storedb -f ${MTLDIR}dbs/user_${TODAY}  -d n_user

cp -f ${DATADIR}photo ${MTLDIR}dbs/photo_${TODAY} &&
/usr/local/bin/nmdb -d /data/qdbm/photo -t 26020 -u 26020 -c 30720 -o /var/log/nmdb.photo &&
${MTLDIR}storedb -f ${MTLDIR}dbs/photo_${TODAY} -d n_photo

cp -f ${DATADIR}video ${MTLDIR}dbs/video_${TODAY} &&
/usr/local/bin/nmdb -d /data/qdbm/video -t 26030 -u 26030 -c 30720 -o /var/log/nmdb.video &&
${MTLDIR}storedb -f ${MTLDIR}dbs/video_${TODAY} -d n_video

cp -f ${DATADIR}blog  ${MTLDIR}dbs/blog_${TODAY}  &&
/usr/local/bin/nmdb -d /data/qdbm/blog -t 26040 -u 26040 -c 30720 -o /var/log/nmdb.blog &&
${MTLDIR}storedb -f ${MTLDIR}dbs/blog_${TODAY}  -d n_blog

