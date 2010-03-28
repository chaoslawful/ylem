dnl $Id$
dnl config.m4 for extension yphp_ylem

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

  PHP_ARG_WITH(yphp_ylem, for yphp_ylem support,
  Make sure that the comment is aligned:
  [  --with-yphp_ylem             Include yphp_ylem support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(yphp_ylem, whether to enable yphp_ylem support,
dnl Make sure that the comment is aligned:
dnl [  --enable-yphp_ylem           Enable yphp_ylem support])

if test "$PHP_YPHP_YLEM" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-yphp_ylem -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/yphp_ylem.h"  # you most likely want to change this
  dnl if test -r $PHP_YPHP_YLEM/$SEARCH_FOR; then # path given as parameter
  dnl   YPHP_YLEM_DIR=$PHP_YPHP_YLEM
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for yphp_ylem files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       YPHP_YLEM_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$YPHP_YLEM_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the yphp_ylem distribution])
  dnl fi

  dnl # --with-yphp_ylem -> add include path
  dnl PHP_ADD_INCLUDE($YPHP_YLEM_DIR/include)

  dnl # --with-yphp_ylem -> check for lib and symbol presence
  dnl LIBNAME=yphp_ylem # you may want to change this
  dnl LIBSYMBOL=yphp_ylem # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $YPHP_YLEM_DIR/lib, YPHP_YLEM_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_YPHP_YLEMLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong yphp_ylem lib version or lib not found])
  dnl ],[
  dnl   -L$YPHP_YLEM_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(YPHP_YLEM_SHARED_LIBADD)
  PHP_SUBST(YPHP_YLEM_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(stdc++,/home/y/lib, YPHP_YLEM_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(ylem_Client, ../../common, YPHP_YLEM_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(log4cpp_y, /home/y/lib, YPHP_YLEM_SHARED_LIBADD)
  PHP_ADD_INCLUDE("/home/y/include/")
  PHP_ADD_INCLUDE("../../common/")

  PHP_NEW_EXTENSION(yphp_ylem, yphp_ylem.cc, $ext_shared)
fi
