dnl $Id$
dnl config.m4 for extension nevent


PHP_ARG_WITH(nevent, for nevent support,
[  --with-nevent   Include nevent support])



if test "$PHP_NEVENT" != "no"; then
  dnl # --with-nevent -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/mevent.h"   # you most likely want to change this
  if test -r $PHP_NEVENT/$SEARCH_FOR; then # path given as parameter
     NEVENT_DIR=$PHP_NEVENT
  else # search default path list
    AC_MSG_CHECKING([for nevent files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        NEVENT_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
 
  if test -z "$NEVENT_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the nevent distribution])
  fi

   
  
 PHP_ADD_INCLUDE($NEVENT_DIR/include)
    
 
   
  LIBNAME=nevent # you may want to change this
  LIBSYMBOL=printf # you most likely want to change this 
 
  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $NEVENT_DIR/lib, NEVENT_SHARED_LIBADD)
    AC_DEFINE(HAVE_NEVENTLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong nevent lib version or lib not found .nevent default dir is $NEVENT_DIR])
  ],[
    -L$NEVENT_DIR/lib -lm -ldl   
  ])
 
  PHP_SUBST(NEVENT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(nevent, nevent.c, $ext_shared)
fi
