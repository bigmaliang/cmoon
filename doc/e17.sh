#!/bin/sh

####################
### enlightenment 17 install script for debian
### create on 2011-03-06
### useage:
###     #mkdir -p /usr/local/src/e; mkdir -p /usr/local/e17; cp e17.sh /usr/local/src/e;
###     #cd /usr/local/src/e/ ; ./e17.sh init
###     #cp -r /usr/local/e17 /usr/local/e17.bak; cd /usr/local/src/e/; ./e17.sh up
### E17 INST: http://www.enlightenment.org/p.php?p=contribute&l=en
### SVN RESP: http://trac.enlightenment.org/e/browser/trunk?order=date
####################

DEBPKGS="liblua5.1-0-dev libexif-dev libcurl4-openssl-dev"
EMODEXCEPT="drawer eweather"

set -e
PREFIX="/usr/local/e17"
PROJECTS="eina eet evas ecore embryo edje e_dbus efreet e \
          elementary ethumb eio ephoto exalt emprint \
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

