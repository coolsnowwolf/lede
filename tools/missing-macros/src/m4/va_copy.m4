# va_copy.m4 serial 1 (js-1.6.20070208)

dnl ##
dnl ##  Check for C99 va_copy() implementation
dnl ##  (and provide fallback implementation if neccessary)
dnl ##
dnl ##  configure.in:
dnl ##    AC_CHECK_VA_COPY
dnl ##  foo.c:
dnl ##    #include "config.h"
dnl ##    [...]
dnl ##    va_copy(d,s)
dnl ##
dnl ##  This check is rather complex: first because we really have to
dnl ##  try various possible implementations in sequence and second, we
dnl ##  cannot define a macro in config.h with parameters directly.
dnl ##

dnl #   test program for va_copy() implementation
changequote(<<,>>)
m4_define(__va_copy_test, <<[
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define DO_VA_COPY(d, s) $1
void test(char *str, ...)
{
    va_list ap, ap2;
    int i;
    va_start(ap, str);
    DO_VA_COPY(ap2, ap);
    for (i = 1; i <= 9; i++) {
        int k = (int)va_arg(ap, int);
        if (k != i)
            abort();
    }
    DO_VA_COPY(ap, ap2);
    for (i = 1; i <= 9; i++) {
        int k = (int)va_arg(ap, int);
        if (k != i)
            abort();
    }
    va_end(ap);
}
int main(int argc, char *argv[])
{
    test("test", 1, 2, 3, 4, 5, 6, 7, 8, 9);
    exit(0);
}
]>>)
changequote([,])

dnl #   test driver for va_copy() implementation
m4_define(__va_copy_check, [
    AH_VERBATIM($1,
[/* Predefined possible va_copy() implementation (id: $1) */
#define __VA_COPY_USE_$1(d, s) $2])
    if test ".$ac_cv_va_copy" = .; then
        AC_TRY_RUN(__va_copy_test($2), [ac_cv_va_copy="$1"])
    fi
])

dnl #   Autoconf check for va_copy() implementation checking
AC_DEFUN([AC_CHECK_VA_COPY],[
  dnl #   provide Autoconf display check message
  AC_MSG_CHECKING(for va_copy() function)
  dnl #   check for various implementations in priorized sequence   
  AC_CACHE_VAL(ac_cv_va_copy, [
    ac_cv_va_copy=""
    dnl #   1. check for standardized C99 macro
    __va_copy_check(C99, [va_copy((d), (s))])
    dnl #   2. check for alternative/deprecated GCC macro
    __va_copy_check(GCM, [VA_COPY((d), (s))])
    dnl #   3. check for internal GCC macro (high-level define)
    __va_copy_check(GCH, [__va_copy((d), (s))])
    dnl #   4. check for internal GCC macro (built-in function)
    __va_copy_check(GCB, [__builtin_va_copy((d), (s))])
    dnl #   5. check for assignment approach (assuming va_list is a struct)
    __va_copy_check(ASS, [do { (d) = (s); } while (0)])
    dnl #   6. check for assignment approach (assuming va_list is a pointer)
    __va_copy_check(ASP, [do { *(d) = *(s); } while (0)])
    dnl #   7. check for memory copying approach (assuming va_list is a struct)
    __va_copy_check(CPS, [memcpy((void *)&(d), (void *)&(s)), sizeof((s))])
    dnl #   8. check for memory copying approach (assuming va_list is a pointer)
    __va_copy_check(CPP, [memcpy((void *)(d), (void *)(s)), sizeof(*(s))])
    if test ".$ac_cv_va_copy" = .; then
        AC_ERROR([no working implementation found])
    fi
  ])
  dnl #   optionally activate the fallback implementation
  if test ".$ac_cv_va_copy" = ".C99"; then
      AC_DEFINE(HAVE_VA_COPY, 1, [Define if va_copy() macro exists (and no fallback implementation is required)])
  fi
  dnl #   declare which fallback implementation to actually use
  AC_DEFINE_UNQUOTED([__VA_COPY_USE], [__VA_COPY_USE_$ac_cv_va_copy],
      [Define to id of used va_copy() implementation])
  dnl #   provide activation hook for fallback implementation
  AH_VERBATIM([__VA_COPY_ACTIVATION],
[/* Optional va_copy() implementation activation */
#ifndef HAVE_VA_COPY
#define va_copy(d, s) __VA_COPY_USE(d, s)
#endif
])
  dnl #   provide Autoconf display result message
  if test ".$ac_cv_va_copy" = ".C99"; then
      AC_MSG_RESULT([yes])
  else
      AC_MSG_RESULT([no (using fallback implementation)])
  fi
])

