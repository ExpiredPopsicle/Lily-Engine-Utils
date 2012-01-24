#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([LILY-ENGINE-UTILS], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([libs/utils/utils.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([malloc.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset sqrt strchr])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
