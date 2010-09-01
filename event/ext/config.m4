dnl $Id$
dnl config.m4 for extension mevent


PHP_ARG_WITH(mevent, for mevent support,
[  --with-mevent   Include mevent support])



if test "$PHP_MEVENT" != "no"; then
  dnl # --with-mevent -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/mevent.h"   # you most likely want to change this
  if test -r $PHP_MEVENT/$SEARCH_FOR; then # path given as parameter
     MEVENT_DIR=$PHP_MEVENT
  else # search default path list
    AC_MSG_CHECKING([for mevent files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        MEVENT_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
 
  if test -z "$MEVENT_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the mevent distribution])
  fi

   
  
 PHP_ADD_INCLUDE($MEVENT_DIR/include)
    
 
   
  LIBNAME=mevent # you may want to change this
  LIBSYMBOL=printf # you most likely want to change this 
 
  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $MEVENT_DIR/lib, MEVENT_SHARED_LIBADD)
    AC_DEFINE(HAVE_MEVENTLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong mevent lib version or lib not found .mevent default dir is $MEVENT_DIR])
  ],[
    -L$MEVENT_DIR/lib -lm -ldl   
  ])
 
  PHP_SUBST(MEVENT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(mevent, mevent.c, $ext_shared)
fi
