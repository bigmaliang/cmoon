#!/bin/sh

# Debian system
sudo apt-get install clearsilver-dev libfcgi-dev libjson0-dev libmemcached-dev libqdbm-dev libevent-dev
sudo apt-get install libmysqlclient-dev libpq-dev libsqlite3-dev

# Other Linux distribution
if [ 1 == 2 ]; then
	PREFIX="/usr/local"
	SRCS="http://www.clearsilver.net/downloads/clearsilver-0.10.5.tar.gz http://www.fastcgi.com/dist/fcgi.tar.gz http://oss.metaparadigm.com/json-c/json-c-0.9.tar.gz http://download.tangent.org/libmemcached-0.38.tar.gz http://blitiri.com.ar/p/nmdb/files/0.22/nmdb-0.22.tar.gz http://qdbm.sourceforge.net/qdbm-1.8.77.tar.gz"
	DIRS="clearsilver-0.10.5 fcgi-2.4.0 json-c-0.9 libmemcached-0.38 nmdb-0.22 qdbm-1.8.77"

	cd ${PREFIX}/src/
	
	for src in $SRCS; do
		wget $src
	done

	for file in *.tar.gz; do
		tar zxvf $file
	done

	for dir in $DIRS; do
		cd $dir
		
		if [ $dir == "clearsilver-0.10.5" ]; then
			./configure --prefix=${PREFIX} --disable-wdb --disable-perl --disable-ruby --disable-java --disable-python && make && make installl
		elif [ $dir == "nmdb-0.22" ]; then
			cd libnmdb && make && make install && cd -
		else
			./configure --prefix=${PREFIX} && make && make install
		fi
		
		cd -
	done
fi
