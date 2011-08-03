#!/bin/sh

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin
TODAY=`date +%Y%m%d`

cd ~/web/moon/

echo "\n******** CLEAN ********"
sleep 2
make clean

echo "\n******** BUILD ********"
sleep 2
make RLS=1 NFCGI=0

echo "\n******** INSTALL ********"
sleep 2
sudo make RLS=1 NFCGI=0 install

echo "\n******** TAR ********"
sleep 2
tar zcvf ${TODAY}_pop.tar.gz /usr/local/lib/mevent_*.so \
    /usr/local/bin/mevent /usr/local/bin/hb /etc/mevent/ \
    /usr/local/moon/

echo "\n******** RESTORE ********"
sleep 2
make clean
make

echo "\n******** DONE ********"
echo "\n** please remember modify "
echo "ALTER USER lcser with password 'xxxxxx';"
echo "/usr/local/moon/pop/config.hdf (tclevel, pass)"
echo "/usr/local/moon/pop/mtls/view_mtls.sql"
echo "/usr/local/moon/pop/mtls/conf/*.conf"
echo "/etc/mevent/server.hdf (tclevel, pass)"
