# serial 1

AC_DEFUN([mfx_PROG_CPPFLAGS], [
AC_MSG_CHECKING([whether the C preprocessor needs special flags])

AC_LANG_CONFTEST([AC_LANG_PROGRAM(
[[#include <limits.h>
#if (32767 >= 4294967295ul) || (65535u >= 4294967295ul)
#  include "your C preprocessor is broken 1"
#elif (0xffffu == 0xfffffffful)
#  include "your C preprocessor is broken 2"
#elif (32767 >= ULONG_MAX) || (65535u >= ULONG_MAX)
#  include "your C preprocessor is broken 3"
#endif
]], [[ ]]
)])

mfx_save_CPPFLAGS=$CPPFLAGS
mfx_tmp=ERROR
for mfx_arg in "" -no-cpp-precomp
do
  CPPFLAGS="$mfx_arg $mfx_save_CPPFLAGS"
  _AC_COMPILE_IFELSE([],
[mfx_tmp=$mfx_arg
break])
done
CPPFLAGS=$mfx_save_CPPFLAGS
rm -f conftest.$ac_ext conftest.$ac_objext
case x$mfx_tmp in
  x)
    AC_MSG_RESULT([none needed]) ;;
  xERROR)
    AC_MSG_RESULT([ERROR])
    AC_MSG_ERROR([your C preprocessor is broken - for details see config.log])
    ;;
  *)
    AC_MSG_RESULT([$mfx_tmp])
    CPPFLAGS="$mfx_tmp $CPPFLAGS"
    ;;
esac
])# mfx_PROG_CPPFLAGS
