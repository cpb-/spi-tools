#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT(spi-tools, 0.8.1, Christophe Blaess)

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE([disable])

##########################################################################
# Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_LANG_C
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_DEFINE([_GNU_SOURCE], 1, [Use GNU extensions])

# Checks for libraries.
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
    then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
#AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([docs/Doxyfile])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memmove memset])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])
AC_CONFIG_HEADERS([src/config.h])

##########################################################################
# debug compilation support
##########################################################################

AC_MSG_CHECKING([whether to build with debug information])
AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug],
                              [enable debug data generation (def=no)])],
                              [debugit="$enableval"],
                              [debugit=no])
AC_MSG_RESULT([$debugit])

if test x"$debugit" = x"yes"; then
    AC_DEFINE([DEBUG],[],[Debug Mode])
    AM_CFLAGS="-g -Wall -Werror -Wno-uninitialized -O0"
    AM_CXXFLAGS="-g -Wall -Werror -Wno-uninitialized -O0"
    AC_MSG_NOTICE([Debug Mode on.])
else
    AC_DEFINE([NDEBUG],[],[No-debug Mode])
    AM_CFLAGS="$AM_CFLAGS -O0"
    AM_CXXFLAGS="$AM_CXXFLAGS -O0"
    AC_MSG_NOTICE([Debug Mode off.])
fi

AC_PATH_PROG(HELP2MAN, help2man, echo No help2man )

CFLAGS="$AM_CFLAGS"
CXXFLAGS="$AM_CXXFLAGS"
##########################################################################

AC_OUTPUT

