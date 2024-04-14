#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([quectel-CM], [1.0], [fae-support@quectel.com])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_ARG_WITH(sanitized-headers,
      AS_HELP_STRING([--with-sanitized-headers=DIR],
         [Specify the location of the sanitized Linux headers]),
      [CPPFLAGS="$CPPFLAGS -idirafter $withval"])

AC_ARG_WITH([qrtr],
       AC_HELP_STRING([--with-qrtr],
          [enable qrtr, building which use qrtr]))

if (test "x${with_qrtr}" = "xyes"); then
	#AC_DEFINE(ENABLE_USEQTRT, 1, [Define if uses qrtr])
	AC_CHECK_HEADERS([linux/qrtr.h linux/rmnet_data.h])
fi

AM_CONDITIONAL(USE_QRTR, test "x${with_qrtr}" = "xyes")

AC_ARG_WITH([msm-ipc],
       AC_HELP_STRING([--with-msm-ipc],
          [enable msm-ipc, building which use qrtr]))

if (test "x${with_msm_ipc}" = "xyes"); then
	#AC_DEFINE(ENABLE_USEQTRT, 1, [Define if uses qrtr])
	AC_CHECK_HEADERS([linux/msm_ipc.h linux/rmnet_data.h])
fi

AM_CONDITIONAL(USE_MSM_IPC, test "x${with_msm_ipc}" = "xyes")

# Checks for library functions.

# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
