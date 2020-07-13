#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# Copyright (C) 2011 Bill Lee <bill.lee.y@gmail.com>
# License: GNU GPL v3 or any later version released by Free Software Foundation

#AC_PREREQ([2.65])
AC_INIT([antiLeech], [44], [https://github.com/persmule/amule-dlp/issues])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR([antiLeech.cpp])
#AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET
AM_DISABLE_STATIC
AC_PROG_LIBTOOL

#Check for wx-config

AC_CHECK_PROG(WXCONFIG, [wx-config], [wx-config])
ANTILEECH_CPPFLAGS=$(wx-config --cppflags)

AC_SUBST(ANTILEECH_CPPFLAGS)

# Checks for libraries.
ANTILEECH_LDFLAGS=

AC_SUBST(ANTILEECH_LDFLAGS)

# Checks for header files.
AC_CHECK_HEADERS([string.h], [wx/string.h], [wx/defs.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
