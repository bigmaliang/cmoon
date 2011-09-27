#!/bin/sh

PWD=`pwd`
DIR=${PWD}../../

echo "sync moon"
cd ${DIR}moon && git st; git pull && git push

echo "sync APE_Server"
cd ${DIR}APE_Server && git st; git pull && git push

echo "sync APE_JSF"
cd ${DIR}APE_JSF && git st; git pull && git push

echo "sync zettair"
cd ${DIR}zettair && git st; git pull && git push
