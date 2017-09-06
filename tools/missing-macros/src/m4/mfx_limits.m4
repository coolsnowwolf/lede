# serial 3

AC_DEFUN([mfx_CHECK_HEADER_SANE_LIMITS_H], [
AC_CACHE_CHECK([whether limits.h is sane],
mfx_cv_header_sane_limits_h,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <limits.h>
#if (32767 >= 4294967295ul) || (65535u >= 4294967295ul)
#  if defined(__APPLE__) && defined(__GNUC__)
#    error "your preprocessor is broken - use compiler option -no-cpp-precomp"
#  else
#    include "your preprocessor is broken"
#  endif
#endif
#define MFX_0xffff          0xffff
#define MFX_0xffffffffL     4294967295ul
#if !defined(CHAR_BIT) || (CHAR_BIT != 8)
#  include "error CHAR_BIT"
#endif
#if !defined(UCHAR_MAX)
#  include "error UCHAR_MAX 1"
#endif
#if !defined(USHRT_MAX)
#  include "error USHRT_MAX 1"
#endif
#if !defined(UINT_MAX)
#  include "error UINT_MAX 1"
#endif
#if !defined(ULONG_MAX)
#  include "error ULONG_MAX 1"
#endif
#if !defined(SHRT_MAX)
#  include "error SHRT_MAX 1"
#endif
#if !defined(INT_MAX)
#  include "error INT_MAX 1"
#endif
#if !defined(LONG_MAX)
#  include "error LONG_MAX 1"
#endif
#if (UCHAR_MAX < 1)
#  include "error UCHAR_MAX 2"
#endif
#if (USHRT_MAX < 1)
#  include "error USHRT_MAX 2"
#endif
#if (UINT_MAX < 1)
#  include "error UINT_MAX 2"
#endif
#if (ULONG_MAX < 1)
#  include "error ULONG_MAX 2"
#endif
#if (UCHAR_MAX < 0xff)
#  include "error UCHAR_MAX 3"
#endif
#if (USHRT_MAX < MFX_0xffff)
#  include "error USHRT_MAX 3"
#endif
#if (UINT_MAX < MFX_0xffff)
#  include "error UINT_MAX 3"
#endif
#if (ULONG_MAX < MFX_0xffffffffL)
#  include "error ULONG_MAX 3"
#endif
#if (USHRT_MAX > UINT_MAX)
#  include "error USHRT_MAX vs UINT_MAX"
#endif
#if (UINT_MAX > ULONG_MAX)
#  include "error UINT_MAX vs ULONG_MAX"
#endif
]], [[
#if (USHRT_MAX == MFX_0xffff)
{ typedef char a_short2a[1 - 2 * !(sizeof(short) == 2)]; }
#elif (USHRT_MAX >= MFX_0xffff)
{ typedef char a_short2b[1 - 2 * !(sizeof(short) > 2)]; }
#endif
#if (UINT_MAX == MFX_0xffff)
{ typedef char a_int2a[1 - 2 * !(sizeof(int) == 2)]; }
#elif (UINT_MAX >= MFX_0xffff)
{ typedef char a_int2b[1 - 2 * !(sizeof(int) > 2)]; }
#endif
#if (ULONG_MAX == MFX_0xffff)
{ typedef char a_long2a[1 - 2 * !(sizeof(long) == 2)]; }
#elif (ULONG_MAX >= MFX_0xffff)
{ typedef char a_long2b[1 - 2 * !(sizeof(long) > 2)]; }
#endif
#if !defined(_CRAY1) /* CRAY PVP systems */
#if (USHRT_MAX == MFX_0xffffffffL)
{ typedef char a_short4a[1 - 2 * !(sizeof(short) == 4)]; }
#elif (USHRT_MAX >= MFX_0xffffffffL)
{ typedef char a_short4b[1 - 2 * !(sizeof(short) > 4)]; }
#endif
#endif /* _CRAY1 */
#if (UINT_MAX == MFX_0xffffffffL)
{ typedef char a_int4a[1 - 2 * !(sizeof(int) == 4)]; }
#elif (UINT_MAX >= MFX_0xffffffffL)
{ typedef char a_int4b[1 - 2 * !(sizeof(int) > 4)]; }
#endif
#if (ULONG_MAX == MFX_0xffffffffL)
{ typedef char a_long4a[1 - 2 * !(sizeof(long) == 4)]; }
#elif (ULONG_MAX >= MFX_0xffffffffL)
{ typedef char a_long4b[1 - 2 * !(sizeof(long) > 4)]; }
#endif
]])],
[mfx_cv_header_sane_limits_h=yes],
[mfx_cv_header_sane_limits_h=no])])
])

# /***********************************************************************
# // standard
# ************************************************************************/

AC_DEFUN([mfx_LZO_CHECK_ENDIAN], [
AC_C_BIGENDIAN([AC_DEFINE(LZO_ABI_BIG_ENDIAN,1,[Define to 1 if your machine is big endian.])],[AC_DEFINE(LZO_ABI_LITTLE_ENDIAN,1,[Define to 1 if your machine is little endian.])])
])#


# /***********************************************************************
# //
# ************************************************************************/

dnl more types which are not yet covered by ACC

AC_DEFUN([mfx_CHECK_SIZEOF], [
AC_CHECK_SIZEOF(__int32)
AC_CHECK_SIZEOF(intmax_t)
AC_CHECK_SIZEOF(uintmax_t)
AC_CHECK_SIZEOF(intptr_t)
AC_CHECK_SIZEOF(uintptr_t)

AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(long double)

AC_CHECK_SIZEOF(dev_t)
AC_CHECK_SIZEOF(fpos_t)
AC_CHECK_SIZEOF(mode_t)
AC_CHECK_SIZEOF(off_t)
AC_CHECK_SIZEOF(ssize_t)
AC_CHECK_SIZEOF(time_t)
])#



AC_DEFUN([mfx_CHECK_LIB_WINMM], [
if test "X$GCC" = Xyes; then
case $host_os in
cygwin* | mingw* | pw32*)
     test "X$LIBS" != "X" && LIBS="$LIBS "
     LIBS="${LIBS}-lwinmm" ;;
*)
     ;;
esac
fi
])#
