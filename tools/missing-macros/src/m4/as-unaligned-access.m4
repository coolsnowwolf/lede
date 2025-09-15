dnl AS_UNALIGNED_ACCESS

dnl check if unaligned memory access works correctly
AC_DEFUN([AS_UNALIGNED_ACCESS], [
  AC_MSG_CHECKING([if unaligned memory access works correctly])
  if test x"$as_cv_unaligned_access" = x ; then
    case $host in
      alpha*|arm*|hp*|mips*|sh*|sparc*|ia64*)
        _AS_ECHO_N([(blacklisted) ])
        as_cv_unaligned_access=no
	;;
      i?86*|x86_64|amd64|powerpc*|m68k*|cris*)
        _AS_ECHO_N([(whitelisted) ])
        as_cv_unaligned_access=yes
	;;
    esac
  else
    _AS_ECHO_N([(cached) ])
  fi
  if test x"$as_cv_unaligned_access" = x ; then
    AC_TRY_RUN([
int main(int argc, char **argv)
{
  char array[] = "ABCDEFGH";
  unsigned int iarray[2];
  memcpy(iarray,array,8);
#define GET(x) (*(unsigned int *)((char *)iarray + (x)))
  if(GET(0) != 0x41424344 && GET(0) != 0x44434241) return 1;
  if(GET(1) != 0x42434445 && GET(1) != 0x45444342) return 1;
  if(GET(2) != 0x43444546 && GET(2) != 0x46454443) return 1;
  if(GET(3) != 0x44454647 && GET(3) != 0x47464544) return 1;
  return 0;
}
    ], as_cv_unaligned_access="yes", as_cv_unaligned_access="no")
  fi
  AC_MSG_RESULT($as_cv_unaligned_access)
  if test "$as_cv_unaligned_access" = "yes"; then
    AC_DEFINE_UNQUOTED(HAVE_UNALIGNED_ACCESS, 1,
      [defined if unaligned memory access works correctly])
  fi
])
