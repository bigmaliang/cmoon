#!/bin/bash

PATH=/usr/local/bin:/usr/local/sbin:/bin:/usr/bin:/usr/sbin

#srcdir="/tmp"
srcdir="/usr/local/src"
pwd=`pwd`

echo "check depend library need for moon"
sleep 1

declare -a Packagename=(
    'clearsilver'
    'libmemcached'
    'libjsonc'

    'libmongo-client'

    'fcgi'
    'libmarkdown');

declare -a Judgement=(
    'test -e "/usr/local/include/ClearSilver/ClearSilver.h"'
    'pkg-config --cflags libmemcached > /dev/null 2>&1'
    'pkg-config --cflags json > /dev/null 2>&1'

    'pkg-config --cflags libmongo-client > /dev/null 2>&1'

    'test -e "/usr/local/lib/libfcgi.a"'
    'test -e "/usr/local/lib/libmarkdown.a"');

declare -a Download=(
    'git clone https://github.com/bigmaliang/clearsilver.git clearsilver'
    'wget https://launchpad.net/libmemcached/1.0/0.53/+download/libmemcached-0.53.tar.gz && mkdir libmemcached && tar zxvf libmemcached-0.53.tar.gz -C libmemcached/ --strip-components=1'
    'git clone https://github.com/jehiah/json-c.git libjsonc'

    'git clone https://github.com/bigmaliang/libmongo-client.git libmongo-client'

    'wget http://www.fastcgi.com/dist/fcgi.tar.gz && mkdir fcgi && tar zxvf fcgi.tar.gz -C fcgi/ --strip-components=1'
    'git clone https://github.com/Orc/discount.git libmarkdown');

declare -a Config=(
    './autogen.sh --disable-wdb --disable-perl --disable-ruby --disable-java --disable-python'
    './configure'
    './autogen.sh && ./configure'

    'autoreconf -i && ./configure'

    './configure'
    './configure.sh');


for (( i = 0 ; i < ${#Packagename[@]} ; i++ ))
do
    echo -n "***$i. ${Packagename[$i]}..."
    sleep 1
    if eval ${Judgement[$i]}
    then
        echo "ok"
    else
        echo "not ok, need install"
        echo "      installing..."
        cd $srcdir
        rm -rf ${Packagename[$i]}*
        eval ${Download[$i]} > ${Packagename[$i]}.log 2>&1 &&
        cd ${Packagename[$i]}
        eval ${Config[$i]} >> ../${Packagename[$i]}.log 2>&1 &&
        make >> ../${Packagename[$i]}.log 2>&1
        make install >> ../${Packagename[$i]}.log 2>&1
        ret=$?
        [ $ret -eq 0 ] && echo "      ok" || echo "      not ok, see $srcdir/${Packagename[$i]}.log for more information"
        [ $ret -ne 0 ] && exit 1
        cd $pwd
    fi
done

echo -n "make sure /usr/local/lib in ld.so.conf..."
if grep '/usr/local/lib' /etc/ld.so.conf
then
    ;
else
    echo "/usr/local/lib" >> /etc/ld.so.conf
    ldconfig
fi
echo "ok"


echo "all done, compile will be ok now"
