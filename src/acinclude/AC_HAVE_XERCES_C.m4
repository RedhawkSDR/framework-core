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

AC_DEFUN([AC_HAVE_XERCES_C], [

AC_ARG_WITH(xerces-prefix, [  --with-xerces-prefix=PFX   Prefix where
Xerces-C is installed (optional)],
            [xerces_prefix="$withval"], [xerces_prefix=""])

AC_LANG_PUSH([C++])
AC_MSG_CHECKING([for Xerces-C])

if test -n "$xerces_prefix" ; then
  CXXFLAGS="$CXXFLAGS -I$xerces_prefix/include"
  LDFLAGS="$LDFLAGS -L$xerces_prefix/lib"
fi

LIBS="-lxerces-c $LIBS"

AC_LINK_IFELSE([
 AC_LANG_PROGRAM(
  [
  #include <xercesc/parsers/XercesDOMParser.hpp>
  #include <iostream>
  XERCES_CPP_NAMESPACE_USE
  ],
  [
  XercesDOMParser* parser = new XercesDOMParser();
  ])],
 [AC_MSG_RESULT([yes])],
 [AC_MSG_FAILURE([missing])

])

AC_LANG_POP
])



