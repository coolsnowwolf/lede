dnl
dnl check for signal type
dnl
dnl AC_RETSIGTYPE isn't quite right, but almost.
dnl
define(TYPE_SIGNAL,[
AC_MSG_CHECKING([POSIX signal handlers])
AC_CACHE_VAL(cv_has_posix_signals,
[AC_TRY_COMPILE(
[#include <sys/types.h>
#include <signal.h>
#ifdef signal
#undef signal
#endif
extern void (*signal ()) ();], [],
cv_has_posix_signals=yes, cv_has_posix_signals=no)])
AC_MSG_RESULT($cv_has_posix_signals)
if test $cv_has_posix_signals = yes; then
   AC_DEFINE(RETSIGTYPE, void, [Return type is void])
   AC_DEFINE(POSIX_SIGTYPE, [], [Have POSIX signals])
else
  if test $ac_cv_type_signal = void; then
     AC_DEFINE(RETSIGTYPE, void, [Return type is void])
  else
     AC_DEFINE(RETSIGTYPE, int, [Return type is int])
  fi
fi])dnl
