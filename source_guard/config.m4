dnl $Id$
dnl config.m4 for extension source_guard

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(source_guard, for source_guard support,
dnl Make sure that the comment is aligned:
dnl [  --with-source_guard             Include source_guard support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(source_guard, whether to enable source_guard support,
dnl Make sure that the comment is aligned:
[  --enable-source_guard           Enable source_guard support])

if test "$PHP_SOURCE_GUARD" != "no"; then
  dnl Write more examples of tests here...

  dnl # get library FOO build options from pkg-config output
  dnl AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  dnl AC_MSG_CHECKING(for libfoo)
  dnl if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists foo; then
  dnl   if $PKG_CONFIG foo --atleast-version 1.2.3; then
  dnl     LIBFOO_CFLAGS=`$PKG_CONFIG foo --cflags`
  dnl     LIBFOO_LIBDIR=`$PKG_CONFIG foo --libs`
  dnl     LIBFOO_VERSON=`$PKG_CONFIG foo --modversion`
  dnl     AC_MSG_RESULT(from pkgconfig: version $LIBFOO_VERSON)
  dnl   else
  dnl     AC_MSG_ERROR(system libfoo is too old: version 1.2.3 required)
  dnl   fi
  dnl else
  dnl   AC_MSG_ERROR(pkg-config not found)
  dnl fi
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBDIR, SOURCE_GUARD_SHARED_LIBADD)
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)

  dnl # --with-source_guard -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/source_guard.h"  # you most likely want to change this
  dnl if test -r $PHP_SOURCE_GUARD/$SEARCH_FOR; then # path given as parameter
  dnl   SOURCE_GUARD_DIR=$PHP_SOURCE_GUARD
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for source_guard files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SOURCE_GUARD_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SOURCE_GUARD_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the source_guard distribution])
  dnl fi

  dnl # --with-source_guard -> add include path
  dnl PHP_ADD_INCLUDE($SOURCE_GUARD_DIR/include)

  dnl # --with-source_guard -> check for lib and symbol presence
  dnl LIBNAME=source_guard # you may want to change this
  dnl LIBSYMBOL=source_guard # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SOURCE_GUARD_DIR/$PHP_LIBDIR, SOURCE_GUARD_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SOURCE_GUARDLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong source_guard lib version or lib not found])
  dnl ],[
  dnl   -L$SOURCE_GUARD_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SOURCE_GUARD_SHARED_LIBADD)

  PHP_NEW_EXTENSION(source_guard, source_guard.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
