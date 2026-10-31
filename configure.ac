# SPDX-License-Identifier: GPL-2.0-only
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([spi-tools], m4_esyscmd_s([cat VERSION]), [Christophe Blaess])
AC_CONFIG_SRCDIR([src/spi-tools.c])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE([disable])

AC_PROG_CC
# Since Autoconf 2.70, AC_PROG_CC checks for C11 and C99 itself.
m4_version_prereq([2.70], [], [AC_PROG_CC_C99])
AC_USE_SYSTEM_EXTENSIONS

AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug], [enable debug checks and compiler warnings])],
    [], [enable_debug=no])
AS_CASE([$enable_debug],
    [yes], [AC_DEFINE([DEBUG], [1], [Enable debug checks])
            DEBUG_CFLAGS="-g -Wall -Wextra -Werror"],
    [no],  [AC_DEFINE([NDEBUG], [1], [Disable debug assertions])
            DEBUG_CFLAGS=""],
    [AC_MSG_ERROR([--enable-debug expects yes or no])])
AC_SUBST([DEBUG_CFLAGS])

AC_ARG_VAR([PYTHON], [Python 3 interpreter for the optional regression tests])
AC_PATH_PROG([PYTHON], [python3])
AM_CONDITIONAL([HAVE_PYTHON], [test -n "$PYTHON" && test "$cross_compiling" != yes])

AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])
AC_OUTPUT
