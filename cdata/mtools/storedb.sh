#!/bin/sh

TODAY=`date +%F`
DATADIR=/data/qdbm/
MTLDIR=/data/www/rtds/mtools/

cp -f ${DATADIR}photo ${MTLDIR}dbs/photo_${TODAY} && ${MTLDIR}storedb -f ${MTLDIR}dbs/photo_${TODAY} -d n_photo
cp -f ${DATADIR}user  ${MTLDIR}dbs/user_${TODAY}  && ${MTLDIR}storedb -f ${MTLDIR}dbs/user_${TODAY}  -d n_user
cp -f ${DATADIR}blog  ${MTLDIR}dbs/blog_${TODAY}  && ${MTLDIR}storedb -f ${MTLDIR}dbs/blog_${TODAY}  -d n_blog
cp -f ${DATADIR}video ${MTLDIR}dbs/video_${TODAY} && ${MTLDIR}storedb -f ${MTLDIR}dbs/video_${TODAY} -d n_video

