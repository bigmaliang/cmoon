#!/bin/sh

####################
### enlightenment 17 install script for debian
### create on 2011-03-06
### useage:
###     #mkdir -p /usr/local/src/e; mkdir -p /usr/local/e; cp e17.sh /usr/local/src/e;
###     #cd /usr/local/src/e/ ; ./e17.sh init
###     OR
###     #cp -r /usr/local/e /usr/local/e.bak; cd /usr/local/src/e/; ./e17.sh up
###     OR
###     #cd /usr/local/src/e/; ./e17.sh
### E17 INST: http://www.enlightenment.org/p.php?p=contribute&l=en
### SVN RESP: http://trac.enlightenment.org/e/browser/trunk?order=date
####################

DEBPKGS="gettext doxygen wpasupplicant autopoint \
         libpam-dev libfreetype6-dev libpng-dev libjpeg-dev \
         libxcursor-dev libxrender-dev libxrandr-dev \
         libxfixes-dev libxdamage-dev libxcomposite-dev \
         libxss-dev libxp-dev libxext-dev libxinerama-dev \
         libxkbfile-dev libxtst-dev libmpd-dev \
         libzzip-dev \
         libdbus-1-dev \
         liblua5.1-0-dev \
         libexif-dev libcurl4-openssl-dev \
         libtiff-dev librsvg2-dev libgif-dev libcurl4-openssl-dev libgnutls-dev"
EMODEXCEPT="drawer eweather"

set -e
PREFIX="/usr/local/e"
#PROJECTS="eina eet evas ecore embryo edje e_dbus efreet e \
#          ethumb eio elementary ephoto exalt emprint \
#          E-MODULES-EXTRA"
PROJECTS="emprint \
          E-MODULES-EXTRA"
SITE="svn.enlightenment.org"
SVN="http://$SITE/svn/e/trunk"
OPT="--prefix=$PREFIX"

### prepare
if [ "$1" = "init" ]; then
  apt-get install $DEBPKGS
  cp /usr/lib/pkgconfig/lua5.1.pc /usr/lib/pkgconfig/lua.pc
fi

### sync
for P in $PROJECTS; do
  if [ "$1" = "up" ]; then
    svn up $P
  elif [ "$1" = "init" ]; then
    svn co $SVN/$P
  fi
done

### except
#don't exit on E-MODULES-EXTRA's sub directory make failure
sed -i 's#$@ || exit 1 ;#$@ || echo "make $$d failure" \>\> makeError ; #g' E-MODULES-EXTRA/Makefile.in
for Q in $EMODEXCEPT; do
  if [ -d "E-MODULES-EXTRA/$Q" ]; then
    rm -rf E-MODULES-EXTRA/.$Q
    mv -f E-MODULES-EXTRA/$Q E-MODULES-EXTRA/.$Q
  fi
done

### make
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH"
export PATH="$PREFIX/bin:$PATH"
export LD_LIBRARY_PATH="$PREFIX/lib:$LD_LIBRARY_PATH"
for PROJ in $PROJECTS; do
  cd $PROJ
  echo "*************** CURRENT ON ***************" `pwd`
  make clean distclean || true
  if [ "$PROJ" = "evas" ]; then
    ./autogen.sh $OPT --enable-xrender-x11
  else
    ./autogen.sh $OPT
  fi
  make
  sudo make install
  cd ../
  sudo ldconfig
done
