dnl GR_LIB64()
dnl
dnl Checks to see if we're on a x86_64 or powerpc64 machine, and if so, determine
dnl if libdir should end in "64" or not.
dnl
dnl Sets gr_libdir_suffix to "" or "64" and calls AC_SUBST(gr_libdir_suffix)
dnl May append "64" to libdir.
dnl
dnl The current heuristic is:
dnl   if the host_cpu isn't x86_64 or powerpc64, then ""
dnl   if the host_os isn't linux, then ""
dnl   if we're cross-compiling, ask the linker, by way of the selected compiler
dnl   if we're x86_64 and there's a /lib64 and it's not a symlink, then "64", else ""
dnl   else ask the compiler
dnl
AC_DEFUN([GR_LIB64],[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([AC_PROG_CXX])

  AC_MSG_CHECKING([gr_libdir_suffix])
  gr_libdir_suffix=""
  AC_SUBST(gr_libdir_suffix)

  case "$host_os" in
    linux*) is_linux=yes ;;
    *)      is_linux=no  ;;
  esac

  if test "$is_linux" = no || test "$host_cpu" != "x86_64" && test "$host_cpu" != "powerpc64"; then
    gr_libdir_suffix=""
  elif test "$cross_compiling" = yes; then
    _GR_LIB64_ASK_COMPILER
  elif test "$host_cpu" = "x86_64"; then
    if test -d /lib64 && test ! -L /lib64; then
      gr_libdir_suffix=64
    fi
  else
    _GR_LIB64_ASK_COMPILER  
  fi
  AC_MSG_RESULT([$gr_libdir_suffix])


  AC_MSG_CHECKING([whether to append 64 to libdir])
  t=${libdir##*/lib}
  if test "$t" != 64 && test "$gr_libdir_suffix" = "64"; then
    libdir=${libdir}64
    AC_MSG_RESULT([yes. Setting libdir to $libdir])
  else
    AC_MSG_RESULT([no])
  fi
])

