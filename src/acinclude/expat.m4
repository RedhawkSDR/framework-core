dnl -------------------------------------------------------- -*- autoconf -*-
dnl Licensed to the Apache Software Foundation (ASF) under one or more
dnl contributor license agreements.  See the NOTICE file distributed with
dnl this work for additional information regarding copyright ownership.
dnl The ASF licenses this file to You under the Apache License, Version 2.0
dnl (the "License"); you may not use this file except in compliance with
dnl the License.  You may obtain a copy of the License at
dnl
dnl     http://www.apache.org/licenses/LICENSE-2.0
dnl
dnl Unless required by applicable law or agreed to in writing, software
dnl distributed under the License is distributed on an "AS IS" BASIS,
dnl WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
dnl See the License for the specific language governing permissions and
dnl limitations under the License.

dnl
dnl OSSIE_TRY_EXPAT_LINK(
dnl      test-message, cache-var-name, hdrs, libs,
dnl      [actions-on-success], [actions-on-failure])
dnl         
dnl Tests linking against expat with libraries 'libs' and includes
dnl 'hdrs', passing message + cache-var-name to AC_CACHE_CHECK.
dnl On success, sets $expat_libs to libs, sets $ossie_have_expat to 1, 
dnl and runs actions-on-success; on failure runs actions-on-failure.
dnl
AC_DEFUN([OSSIE_TRY_EXPAT_LINK], [
AC_CACHE_CHECK([$1], [$2], [
  ossie_expat_LIBS=$LIBS
  LIBS="$LIBS $4"

  AC_TRY_LINK([#include <stdlib.h>
  #include <$3>], [XML_ParserCreate(NULL);],
      [$2=yes], [$2=no])

  AC_TRY_LINK([#include <stdlib.h>
  #include <$3>], [XML_GetParsingStatus(NULL, NULL);],
      [$2=yes], [$2=no])

  LIBS=$ossie_expat_LIBS
])

if test $[$2] = yes; then
   AC_DEFINE([HAVE_]translit([$3], [a-z./], [A-Z__]), 1,
             [Define if $3 is available])
   ossie_expat_libs="$4"
   ossie_has_expat=1
   $5
else
   ossie_has_expat=0
   $6
fi
])

dnl
dnl OSSIE_SYSTEM_EXPAT: tests for a system expat installation
dnl If present, sets $ossie_has_expat to 1 and adjusts LDFLAGS/CPPFLAGS
dnl appropriately.  This is mostly for compatibility with existing
dnl expat releases; all but the first OSSIE_TRY_EXPAT_LINK call could
dnl be dropped later.
dnl
AC_DEFUN([OSSIE_SYSTEM_EXPAT], [
 
  OSSIE_TRY_EXPAT_LINK([Expat 1.95.8+], ossie_cv_expat_system, 
    [expat.h], [-lexpat])

  if test $ossie_has_expat = 0; then
    OSSIE_TRY_EXPAT_LINK([old Debian-packaged expat], ossie_cv_expat_debian,
       [xmltok/xmlparse.h], [-lxmlparse -lxmltok])
  fi

  if test $ossie_has_expat = 0; then
    OSSIE_TRY_EXPAT_LINK([old FreeBSD-packaged expat], ossie_cv_expat_freebsd,
       [xml/xmlparse.h], [-lexpat])
  fi

  if test $ossie_has_expat = 0; then
    OSSIE_ADDTO(LDFLAGS, [-L/usr/local/lib])
    OSSIE_ADDTO(CPPFLAGS, [-I/usr/local/include])
 
    OSSIE_TRY_EXPAT_LINK([Expat 1.95.8+ in /usr/local], 
       ossie_cv_expat_usrlocal, [expat.h], [-lexpat],
       [OSSIE_ADDTO(EXPAT_CPPFLAGS, [-I/usr/local/include])
        OSSIE_ADDTO(EXPAT_LIBS, [-L/usr/local/lib])],[
       OSSIE_REMOVEFROM(LDFLAGS, [-L/usr/local/lib])
       OSSIE_REMOVEFROM(CPPFLAGS, [-I/usr/local/include])
      ])
  fi
])


dnl
dnl OSSIE_FIND_EXPAT: figure out where EXPAT is located (or use bundled)
dnl
AC_DEFUN([OSSIE_FIND_EXPAT], [

save_cppflags="$CPPFLAGS"
save_ldflags="$LDFLAGS"

ossie_has_expat=0

AC_ARG_WITH([expat],
[  --with-expat=DIR        specify Expat location], [
  if test "$withval" = "yes"; then
    AC_MSG_ERROR([a directory must be specified for --with-expat])
  elif test "$withval" = "no"; then
    AC_MSG_ERROR([Expat cannot be disabled (at this time)])
  else
    # Add given path to standard search paths if appropriate:
    if test "$withval" != "/usr"; then
      OSSIE_ADDTO(LDFLAGS, [-L$withval/lib])
      OSSIE_ADDTO(CPPFLAGS, [-I$withval/include])
      OSSIE_ADDTO(EXPAT_INCLUDES, [-I$withval/include])
      OSSIE_ADDTO(EXPAT_LIBS, [-L$withval/lib])
    fi
  fi
])

OSSIE_SYSTEM_EXPAT

if test "${ossie_has_expat}" = "0"; then
  AC_MSG_ERROR("failed to find expat 1.95.8+")
else
  OSSIE_ADDTO(EXPAT_LIBS, [$ossie_expat_libs])

  OSSIE_XML_DIR=$bundled_subdir
  AC_SUBST(OSSIE_XML_DIR)
  AC_SUBST(EXPAT_CPPFLAGS)
  AC_SUBST(EXPAT_LIBS)

  CPPFLAGS=$save_cppflags
  LDFLAGS=$save_ldflags
fi

])


dnl
dnl OSSIE_ADDTO(variable, value)
dnl
dnl  Add value to variable
dnl
AC_DEFUN([OSSIE_ADDTO], [
  if test "x$$1" = "x"; then
    test "x$silent" != "xyes" && echo "  setting $1 to \"$2\""
    $1="$2"
  else
    ossie_addto_bugger="$2"
    for i in $ossie_addto_bugger; do
      ossie_addto_duplicate="0"
      for j in $$1; do
        if test "x$i" = "x$j"; then
          ossie_addto_duplicate="1"
          break
        fi
      done
      if test $ossie_addto_duplicate = "0"; then
        test "x$silent" != "xyes" && echo "  adding \"$i\" to $1"
        $1="$$1 $i"
      fi
    done
  fi
])dnl

dnl
dnl OSSIE_REMOVEFROM(variable, value)
dnl
dnl Remove a value from a variable
dnl
AC_DEFUN([OSSIE_REMOVEFROM], [
  if test "x$$1" = "x$2"; then
    test "x$silent" != "xyes" && echo "  nulling $1"
    $1=""
  else
    ossie_new_bugger=""
    ossie_removed=0
    for i in $$1; do
      if test "x$i" != "x$2"; then
        ossie_new_bugger="$ossie_new_bugger $i"
      else
        ossie_removed=1
      fi
    done
    if test $ossie_removed = "1"; then
      test "x$silent" != "xyes" && echo "  removed \"$2\" from $1"
      $1=$ossie_new_bugger
    fi
  fi
]) dnl
