#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

dnl AC_PROG_JAVA([min-version], [vendor])
AC_DEFUN([AC_PROG_JAVA],
[
  AC_PATH_PROG([JAVA], [java], [no])
])

dnl AC_PROG_JAVAC([compliance, [vendor]])
AC_DEFUN([AC_PROG_JAVAC],
[
  AC_PATH_PROG([JAVAC], [javac], [no])
  if test x"$1" != "x" -a x"$JAVAC" != "x"; then
    AC_MSG_CHECKING([javac version $1 compliance])
    cat << EOF > Test.java
      import java.util.Properties;
      public class Test {
	public static void main(String[[]] args) {
	}
      }
EOF

    if AC_TRY_COMMAND([$JAVAC -source $1 -target $1 Test.java]); then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      JAVAC=no
    fi 
    rm -f Test.java Test.class
  fi
])

AC_DEFUN([AC_PROG_JAR],
[
  AC_PATH_PROG([JAR], [jar], [no])
])

AC_DEFUN([AC_PROG_IDLJ],
[
  AC_REQUIRE([AC_PROG_JAVAC])
  AC_PATH_PROG(BASENAME, basename)
  AC_PATH_PROG(READLINK, readlink)
  AS_IF([test "x$JAVAC" != "xno"], [
    AS_IF([test "x$BASENAME" != "x"], [
      AS_IF([test "x$READLINK" != "x"], [
        javac_abs_path=`$READLINK -f $JAVAC`
        appendpath=`dirname $javac_abs_path`
      ])
    ])
  ])
  
  AC_PATH_PROG([IDLJ], [idlj], [no], ${PATH}${PATH_SEPARATOR}${appendpath})
])

dnl Check the value of a java property
dnl GET_JAVA_PROPERTY( VARIABLE, PROPNAME)
AC_DEFUN([GET_JAVA_PROPERTY],
[
AC_REQUIRE([AC_PROG_JAVA])
AC_REQUIRE([AC_PROG_JAVAC])
AC_CACHE_CHECK([for java property $2],
$1,
[
  cat << EOF > Test.java
    import java.util.Properties;
    public class Test {
      public static void main(String[[]] args) {
       Properties props = System.getProperties();
       System.out.println(props.getProperty("$2"));
      }
    }
EOF
  $JAVAC Test.java &> /dev/null
  $1=`$JAVA -cp . Test`
  rm -f Test.java Test.class
])
AC_DEFINE($1, cv_java_prop_$1)
])
