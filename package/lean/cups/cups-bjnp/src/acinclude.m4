dnl
dnl Contains the following macros
dnl   BJNP_CHECK_IPV6
dnl

#
# Check for AF_INET6, determines whether or not to enable IPv6 support
# Check for ss_family member in struct sockaddr_storage
# Taken from sane-backends
AC_DEFUN([BJNP_CHECK_IPV6],
[
  AC_MSG_CHECKING([whether to enable IPv6]) 
  AC_ARG_ENABLE(ipv6, 
    AC_HELP_STRING([--disable-ipv6],[disable IPv6 support]), 
      [  if test "$enableval" = "no" ; then
         AC_MSG_RESULT([no, manually disabled]) 
         ipv6=no 
         fi
      ])

  if test "$ipv6" != "no" ; then
    AC_TRY_COMPILE([
	#define INET6 
	#include <sys/types.h> 
	#include <sys/socket.h> ], [
	 /* AF_INET6 available check */  
 	if (socket(AF_INET6, SOCK_STREAM, 0) < 0) 
   	  exit(1); 
 	else 
   	  exit(0); 
      ],[
        AC_MSG_RESULT(yes) 
        AC_DEFINE([ENABLE_IPV6], 1, [Define to 1 if the system supports IPv6]) 
        ipv6=yes
      ],[
        AC_MSG_RESULT([no (couldn't compile test program)]) 
        ipv6=no
      ])
  fi

  if test "$ipv6" != "no" ; then
    AC_MSG_CHECKING([whether struct sockaddr_storage has an ss_family member])
    AC_TRY_COMPILE([
	#define INET6
	#include <sys/types.h>
	#include <sys/socket.h> ], [
	/* test if the ss_family member exists in struct sockaddr_storage */
	struct sockaddr_storage ss;
	ss.ss_family = AF_INET;
	exit (0);
    ], [
	AC_MSG_RESULT(yes)
	AC_DEFINE([HAS_SS_FAMILY], 1, [Define to 1 if struct sockaddr_storage has an ss_family member])
    ], [
		AC_TRY_COMPILE([
		#define INET6
		#include <sys/types.h>
		#include <sys/socket.h> ], [
		/* test if the __ss_family member exists in struct sockaddr_storage */
		struct sockaddr_storage ss;
		ss.__ss_family = AF_INET;
		exit (0);
	  ], [
		AC_MSG_RESULT([no, but __ss_family exists])
		AC_DEFINE([HAS___SS_FAMILY], 1, [Define to 1 if struct sockaddr_storage has __ss_family instead of ss_family])
	  ], [
		AC_MSG_RESULT([no])
		ipv6=no
    	  ])
    ])
  fi	
])

