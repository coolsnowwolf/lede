dnl Process this file with autoconf to produce a configure script.
AC_INIT([cups-bjnp], [2.0.2], [louis.lagendijk@gmail.com])

AC_CONFIG_SRCDIR([bjnp.c])
AM_INIT_AUTOMAKE([gnu])
AC_LIBSOURCES([cups-bjnp.spec])

## Check for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_CC_C_O
##
## Library directories
##
SRCH_LIB="/usr/local/lib /usr/lib /opt/cups/lib /opt/lib"
ac_save_IFS=$IFS
IFS="${IFS}:"
## LIBRARY_DIRS comes from command line, SRCH_LIB s defined above.
for dir in $LIBRARY_DIRS $SRCH_LIB; do
  if test -d "$dir"; then
    LIBDIRS="$LIBDIRS -L$dir"
## else
##    AC_MSG_WARN([*** Library directory $dir does not exist.])
  fi
done
IFS=$ac_save_IFS

dnl Checks for required library functions.
AC_CHECK_FUNCS(ftime \
               getaddrinfo \
               getnameinfo \
               gethostname \
               inet_ntoa \
               inet_ntop \
               ntohl \
               ntohs \
               memset\
               select \
               socket \
               strcasecmp \
               strchr \
               strerror \
               strncasecmp \
	       strstr \
	       strtol \
               ,,AC_MSG_ERROR( required library function missing ))

AC_FUNC_MALLOC

dnl check for optional functions
AC_CHECK_FUNCS(getifaddrs)

dnl check for Werror

AC_ARG_ENABLE(Werror,
        AC_HELP_STRING([--disable-Werror], [don't use gcc's -Werror option when building]))

if test x"$enable_Werror" != xno
then
        EXTRA_CFLAGS="-Werror"
fi
AC_SUBST([EXTRA_CFLAGS])

dnl check for IPv6 (can be overriden by --enable-ipv6)
if test "$ac_cv_func_getnameinfo" = "yes" \
  && test "$ac_cv_func_getaddrinfo" = "yes" ; then
  BJNP_CHECK_IPV6
else
  ipv6="no"
fi

## find cups-configuration
AC_MSG_CHECKING(cups development support)
if test -n "`cups-config --build 2> /dev/null`"; then
	cups_config_found=yes
        AC_MSG_RESULT($cups_config_found)
else
	AC_MSG_ERROR([cups development files not found])
fi

## determine cups backend directory

AC_MSG_CHECKING([cups backend directory])
AC_ARG_WITH(cupsbackenddir,
	    AC_HELP_STRING([--with-cupsbackenddir=DIR],
                           [cups-backends directory (auto)]),
	    [cupsbackenddir="${withval}"],
	    [cupsbackenddir=`cups-config --serverbin`/backend])
if test -d "$cupsbackenddir"; then
	AC_SUBST([cupsbackenddir])
	AC_MSG_RESULT([$cupsbackenddir])
else
	AC_MSG_ERROR([cups backend directory not found!])
fi
AC_SUBST([CUPS_CFLAGS], ["`cups-config --cflags`"])
AC_SUBST([CUPS_LIBS], ["`cups-config --libs`"])
AC_SUBST([CUPS_LDFLAGS], ["`cups-config --ldflags`"])

## header file that contains output from configure

AC_CONFIG_HEADERS([config.h])
## AC_USE_SYSTEM_EXTENSIONS
dnl Checks for programs.
AC_SEARCH_LIBS([strerror],[cposix])

## determine libraries that need to get linked

AC_SEARCH_LIBS([socket], [socket])

## Checks for header files.
AC_HEADER_STDC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_HEADERS(string.h \
                 fcntl.h \
                 arpa/inet.h \
                 netdb.h \
                 netinet/in.h \
                 sys/socket.h \
                 sys/time.h \
                 sys/timeb.h \
                 wchar.h \
                 cups/cups.h \
		 cups/backend.h \
                 cups/http.h \
                ,, AC_MSG_ERROR( required header file missing ))

AC_HEADER_RESOLV

AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

## dnl internationalization macros
## AM_GNU_GETTEXT
AM_MAINTAINER_MODE
## AC_CONFIG_FILES([Makefile])
AC_OUTPUT([Makefile])
