## configure.ac - Autotools configuration file for Crypto++.
##                written and placed in public domain by Jeffrey Walton.
##                based on Debian configure.ac by László Böszörményi.


## Visit the link below for the original Debian Autotools files
##   http://sources.debian.net/src/libcrypto++/5.6.4-8/debian/autotools/
##
## To generate the configure file perform the following. The extra steps are
## needed because the Autotools on some machines, like a PowerMac or Solaris,
## are not modern enough for macros like AC_PROG_LIBTOOL and AC_ENABLE_SHARED.
##
##   autoupdate && libtoolize && autoreconf
##
## Our Autoconf tests are strict. We use custom AC_COMPILE_IFELSE and
## AC_LINK_IFELSE that rejects a feature test if there are any warnings or
## errors. We needed this because Autotools was mis-detecting features on AIX
## and Solarix using XLC and SunCC. We also needed the custom tests because
## Autoconf does not have a "warnings as errors" option that just works on all
## the compilers we support.


## TODO
##
## - Figure out how to make Autoconf stop using the fucking C compiler for tasks.
##   The damn tool does not honor AC_PROG_CXX or observe the absence of
##   AC_PROG_CC. It also fails to provide a AC_PROG_NO_CC macro. The damn tool
##   is no better than Cmake.
##
## - Fix Autotools not treating warnings as errors. Our test programs are clean,
##   so they should compile cleanly or the feature is not available. However,
##   Autotools lacks a AC_TREAT_WARNINGS_AS_ERRORS-like macro or define.
##
## - Fix Autoconf using the wrong AR and ARFLAGS on platforms like OS X. Also
##   see http://lists.gnu.org/archive/html/bug-autoconf/2017-11/msg00002.html.
##
## - Fix Autoconf not honoring LT_INIT. We are trying to enable PIC, static and
##   shared by default but Autotools is ignoring [pic-only]. Also see
##   https://stackoverflow.com/q/56215237/608639.
##
## - Fix Autoconf not using PIC/PIE for cryptest. The problem is bigger than the
##   LT_INIT bug. There is no Uniform Name like bin_LTPROGRAMS to signal PIC/PIE
##   should be used. Autoconf is guessing wrong and not using PIC/PIE.
##
## - Fix Autotools 'make dist' failures. If you run 'make dist' on x86_64, then
##   the release tarball only works on x86_64. It is like it is pre-configured
##   for only the x86_64 platform, and other platform code paths have been
##   thrown away. WTF???
##
## - Fix Autoconf ignoring our config.h.in. This one looks like it is a problem
##   without a solution. Also see https://stackoverflow.com/q/48436715/608639.
##   (Gave up after https://github.com/weidai11/cryptopp/commit/c51f0ecbfd21.
##   Now we can write new config_asm.h, config_cxx.h, and config_misc.h.)
##
## - Figure out how to write new configuration information to config_asm.h,
##   config_cxx.h, and config_misc.h. According to the Autoconf manual we
##   can use multiple headers. However, it does not seem to work in
##   practice. Also see https://stackoverflow.com/q/56193203/608639.
##
## - Stop using local.h, and write the config data into a Crypto++ compatible
##   config.h. There's not much to "Crypto++ compatible config.h". We need test
##   and write preprocessor macros like CRYPTOPP_BIG_ENDIAN,
##   CRYPTOPP_LITTLE_ENDIAN, CRYPTOPP_CXX11, etc. There's just a lot of them.
##   This problem now appears to be closely related to the config.h.in problem.
##   We can't supply a template and expect Autotools to honor it. Also see
##   https://lists.gnu.org/archive/html/autoconf/2018-07/msg00014.html.
##
## - Fix incorrect use of --with-sysroot. Autotools does not seem to honor it
##   when it is set. Also see http://stackoverflow.com/q/47226389/608639.
##
## - Fix MSYS2 compile error due to `lt-cryptest.c` and `lt-cryptestcwd.c`.
##   They are C files and don't use necessary C++ flags. They also use Posix
##   functions but fail to define _XOPEN_SOURCE. I'm guessing this has something
##   to do with libtool, but I have no idea how to fix it at the moment. Also
##   see http://lists.gnu.org/archive/html/bug-libtool/2017-11/msg00002.html.
##
## - Fix the ARM 32-builds. Running 'cryptest v' results in
##   "unexpected reloc type 0x03". GNUmakefile and CMake are OK. I've been
##   working on it (on and off) for three days and no closer to finding a
##   solution. Autotools is such a broken heaping pile of shit... Also see
##   https://sourceware.org/ml/binutils/2019-05/msg00287.html
##
## - Fix static initialization order fiasco tests. We don't know how to craft
##   the tests because we don't know how to compile two files at once using
##   Autotools. Also see https://stackoverflow.com/q/51662985/608639.
##
## - Fix -xarch for SHA when using SunCC. We don't know the arch value at the
##   moment. If things work as expected we will fail the feature test and the
##   code paths will be disabled. But we want them enabled once Sun supports
##   the SHA extensions. See https://stackoverflow.com/q/45872180/608639.
##
## - Fix LDFLAGS on AIX. Autotools is adding unwanted flags to LDFLAGS and it
##   is causing a runtime crash in startup code. See
##   https://stackoverflow.com/q/51680113/608639.
##
## - Add ubsan and asan targets for testing.
##
## - Setup test environment with Dash, which appears to be the lamest shell.
##
## - Package Autotools propery, for a regular user's definition of "proper".
##   I don't know what the final deliverables or artifacts are supposed to
##   look like. We also get warnings and errors due to a missing m4/ directory
##   even though we don't use it. Maybe we need to distribute an empty m4/
##   directory to keep the tools quiet.

#############################################################################

## This configure.ac proceeds in five parts:
##
##   1. configure.ac preamble
##
##   2. compiler and host detection
##
##   3. configure options, like --disable-asm
##
##   4. CPU feature tests, like SSE, NEON, ARMv8, PowerPC
##
##   5. C++ feature tests, like C++11, alignof and alignas
##
##   6. Misc feature and options
##

#############################################################################
##   PART 1   PART 1   PART 1   PART 1   PART 1   PART 1   PART 1   PART 1
## ==========================================================================
## configure.ac preamble
#############################################################################

AC_INIT([Crypto++], [8.3], [http://www.cryptopp.com/wiki/Bug_Report], [cryptopp], [http://www.cryptopp.com])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([local.h])
AC_CONFIG_FILES([Makefile] [libcryptopp.pc])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

AM_PROG_AS
AC_PROG_LIBTOOL

AC_PROG_CXX
AC_LANG([C++])
LT_LANG([C++])

# Programs we use
AC_PROG_SED
AC_PROG_AWK
AC_PROG_GREP
AC_PROG_EGREP

# Detect build and host platforms
AC_CANONICAL_HOST
AC_CANONICAL_BUILD

## Assert this once to avoid surprises
AC_LANG_ASSERT([C++])

#############################################################################

## Our Autoconf tests are strict. We use custom AC_COMPILE_IFELSE and
## AC_LINK_IFELSE that rejects a feature test if there are any warnings or
## errors. We needed this because Autotools was mis-detecting features on
## AIX and Solarix using XLC and SunCC. We also needed the custom tests
## because Autoconf does not have a "warnings as errors" option that just
## works on all the compilers we support.

AC_DEFUN([XXX_COMPILE_IFELSE],
   [AC_COMPILE_IFELSE(
      [$1],
      [retval=0
         touch conftest.err
         if test `cat conftest.err | wc -w` != "0"; then retval=1; fi
      ],[retval=1]
   )
   AS_IF([test $retval = 0],[$2],[$3])]
)

AC_DEFUN([XXX_LINK_IFELSE],
   [AC_LINK_IFELSE(
      [$1],
      [retval=0
         touch conftest.err
         if test `cat conftest.err | wc -w` != "0"; then retval=1; fi
      ],[retval=1]
   )
   AS_IF([test $retval = 0],[$2],[$3])]
)

#############################################################################
##   PART 2   PART 2   PART 2   PART 2   PART 2   PART 2   PART 2   PART 2
## ==========================================================================
## compiler and host detection
#############################################################################

## These are some variables that help us track platform and compiler.
## It speeds up configuration by adding some intelligence to the
## test selection process. It also avoids failed configurations by
## avoiding some tests that fail but Autotools reports as success.

IS_AIX_OS=`echo $build | $GREP -i -c 'aix'`
IS_SUN_OS=`uname -s | $GREP -i -c 'SunOS'`
IS_LINUX_OS=`echo "$build" | $GREP -i -c 'linux'`
IS_APPLE_OS=`echo "$build" | $EGREP -i -c 'apple|darwin'`
IS_ANDROID_OS=`echo $build | $GREP -i -c 'android'`
IS_CYGWIN_OS=`echo $build | $GREP -i -c 'cygwin'`
IS_MINGW_OS=`echo $build | $GREP -i -c 'mingw'`

IS_IA32=`echo $build | $EGREP -i -c 'i.86|x86_64|amd64'`
IS_ARM32=`echo $build | $EGREP -i -c 'arm|armhf|arm7l|eabihf'`
IS_ARMV8=`echo $build | $EGREP -i -c 'aarch32|aarch64'`
IS_PPC=`echo $build | $EGREP -i -c 'ppc|powerpc|powermac'`
IS_SPARC=`echo $build | $EGREP -i -c 'sparc|sparcv8|sparcv9|sparc64'`

## Debug builds add more goodies to help break things. I like breaking things.
IS_DEBUG=`echo $CPPFLAGS $CXXFLAGS | $GREP -i -c '\-DDEBUG'`

#############################################################################

## Determine the compiler's target. GCC '-dumpmachine' is the target,
## so it represents what the compiler is building for (and not what its
## running on). We add '2>/dev/null' because IBM's xlC dumps the man page
## when presented with '-dumpmachine' since it is not a valid option.

## See if CXX is valid. Autotools just trucks on with an invalid one.

if test x"$CXX" != "x"; then
   COMPILER_VALID=`command -v "$CXX"`
   if test x"$COMPILER_VALID" = "x"; then
      AC_MSG_FAILURE(["$CXX" is not a valid compiler or is not on-path])
   fi
fi

## Compiler is valid. Try to detect the target and vendor.
## SunCC is empty, so we just use $build

COMPILER_TARGET=`"$CXX" "$CXXFLAGS" -dumpmachine 2>/dev/null`
if test x"$COMPILER_TARGET" = "x"; then
   COMPILER_TARGET="$build"
fi

## Determine the compiler's vendor. We need to work around some Autoconf bugs.

COMPILER_VERSION=`"$CXX" --version 2>/dev/null | head -n 1`

## IBM xlC test if COMPILER_VERSION is empty
if test x"$COMPILER_VERSION" = "x"; then
   COMPILER_VERSION=`"$CXX" -qversion 2>/dev/null | head -n 1`
fi

## SunCC test if COMPILER_VERSION is empty
if test x"$COMPILER_VERSION" = "x"; then
   COMPILER_VERSION=`"$CXX" -V 2>&1 | head -n 1`
fi

## Microsoft test if COMPILER_VERSION is empty
if test `basename "$CXX"` = "cl.exe"; then
   COMPILER_VERSION=`"$CXX" /? 2>&1 | head -n 1`
fi

IS_GCC_COMPILER=`echo $COMPILER_VERSION | $GREP -i -v 'Clang' | $EGREP -i -c 'GCC|G\+\+'`
IS_MSC_COMPILER=`echo $COMPILER_VERSION | $GREP -i -c 'Microsoft'`
IS_LLVM_COMPILER=`echo $COMPILER_VERSION | $EGREP -i -c 'LLVM|Clang'`
IS_SUN_COMPILER=`echo $COMPILER_VERSION | $EGREP -i -c 'Sun C'`
IS_ICC_COMPILER=`echo $COMPILER_VERSION | $GREP -i -c 'ICC'`
IS_XLC_COMPILER=`echo $COMPILER_VERSION | $GREP -i -c 'IBM XL'`
IS_PORTS_COMPILER=`echo $COMPILER_VERSION | $EGREP -i -c 'MacPorts|Homebrew'`

## A catch-all for compilers that claim to be GCC compatible. Each should be
## able to handle GCC syntax. It excludes XLC, SunCC and MSVC
if test "$IS_GCC_COMPILER" != "0" || test "$IS_LLVM_COMPILER" != "0" || \
   test "$IS_ICC_COMPILER" != "0"; then
   IS_GNUC_COMPILER=1
else
   IS_GNUC_COMPILER=0
fi

## Friendly names for reporting
CXXNAME=`basename "$CXX"`
LDNAME=`basename "$LD"`

## Small fixup
case "$LDNAME" in
    "ld "*) LDNAME=ld
    ;;
esac

#############################################################################
##   PART 3   PART 3   PART 3   PART 3   PART 3   PART 3   PART 3   PART 3
## ==========================================================================
## configure options, like --disable-asm
#############################################################################

AC_ARG_ENABLE([pthreads],
   AS_HELP_STRING([--disable-pthreads], [disable pthreads]))
AS_IF([test "x$enable_pthreads" = "xno"], [
   : # AC_DEFINE([CRYPTOPP_DISABLE_PTHREADS], [1], [disable pthreads])
])

# Applies to all platforms
AC_ARG_ENABLE([asm],
   AS_HELP_STRING([--disable-asm], [disable all ASM and intrinsics]))
AS_IF([test "x$enable_asm" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_ASM], [1], [disable all ASM and intrinsics])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ASM"
])

# X86 defines
AC_ARG_ENABLE([sse2],
   AS_HELP_STRING([--disable-sse2], [disable x86 SSE2 instructions]))
AS_IF([test "x$enable_sse2" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SSE2], [1], [disable x86 SSE2 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE2"
])

AC_ARG_ENABLE([sse3],
   AS_HELP_STRING([--disable-sse3], [disable x86 SSE3 instructions]))
AS_IF([test "x$enable_sse3" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SSE3], [1], [disable x86 SSE3 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE3"
])

AC_ARG_ENABLE([ssse3],
   AS_HELP_STRING([--disable-ssse3], [disable x86 SSSE3 instructions]))
AS_IF([test "x$enable_ssse3" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SSSE3], [1], [disable x86 SSSE3 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSSE3"
])

AC_ARG_ENABLE([sse4],
   AS_HELP_STRING([--disable-sse4], [disable x86 SSE4.1 and SSE4.2 instructions]))
AS_IF([test "x$enable_sse4" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SSE4], [1], [disable x86 SSE4.1 and SSE4.2 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE4"
])

AC_ARG_ENABLE([clmul],
   AS_HELP_STRING([--disable-clmul], [disable x86 CLMUL instructions]))
AS_IF([test "x$enable_clmul" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_CLMUL], [1], [disable x86 CLMUL instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_CLMUL"
])

AC_ARG_ENABLE([aes],
   AS_HELP_STRING([--disable-aes], [disable x86 AES instructions]))
AS_IF([test "x$enable_aes" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_AES], [1], [disable x86 AES instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AES"
])

AC_ARG_ENABLE([sha],
   AS_HELP_STRING([--disable-sha], [disable x86 SHA instructions]))
AS_IF([test "x$enable_sha" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SHA], [1], [disable x86 SHA instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHA"
])

AC_ARG_ENABLE([avx],
   AS_HELP_STRING([--disable-avx], [disable x86 AVX instructions]))
AS_IF([test "x$enable_avx" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_AVX], [1], [disable x86 AVX instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AVX"
])

AC_ARG_ENABLE([avx2],
   AS_HELP_STRING([--disable-avx2], [disable x86 AVX2 instructions]))
AS_IF([test "x$enable_avx2" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_AVX2], [1], [disable x86 AVX2 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AVX2"
])

# ARM-32 defines
AC_ARG_ENABLE([armv7],
   AS_HELP_STRING([--disable-armv7], [disable ARM ARMv7 instructions]))
AS_IF([test "x$enable_armv7" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_ARMV7], [1], [disable ARM ARMv7 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARMV7"
])

AC_ARG_ENABLE([neon],
   AS_HELP_STRING([--disable-neon], [disable ARM NEON instructions]))
AS_IF([test "x$enable_neon" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_NEON], [1], [disable ARM NEON instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_NEON"
])

# ARMv8 defines
AC_ARG_ENABLE([asimd],
   AS_HELP_STRING([--disable-asimd], [disable ARMv8 ASIMD instructions]))
AS_IF([test "x$enable_asimd" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_ASIMD], [1], [disable ARMv8 ASIMD instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ASIMD"
])

AC_ARG_ENABLE([crc32],
   AS_HELP_STRING([--disable-crc32], [disable ARMv8 CRC32 instructions]))
AS_IF([test "x$enable_crc32" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_CRC32], [1], [disable ARMv8 CRC32 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_CRC32"
])

AC_ARG_ENABLE([pmull],
   AS_HELP_STRING([--disable-pmull], [disable ARMv8 PMULL instructions]))
AS_IF([test "x$enable_pmull" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_PMULL], [1], [disable ARMv8 PMULL instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_PMULL"
])

AC_ARG_ENABLE([aes],
   AS_HELP_STRING([--disable-aes], [disable ARMv8 AES instructions]))
AS_IF([test "x$enable_aes" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_AES], [1], [disable ARMv8 AES instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AES"
])

AC_ARG_ENABLE([sha1],
   AS_HELP_STRING([--disable-sha1], [disable ARMv8 SHA1 instructions]))
AS_IF([test "x$enable_sha1" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SHA1], [1], [disable ARMv8 SHA1 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHA1"
])

AC_ARG_ENABLE([sha256],
   AS_HELP_STRING([--disable-sha256], [disable ARMv8 SHA256 instructions]))
AS_IF([test "x$enable_sha256" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SHA256], [1], [disable ARMv8 SHA256 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHA256"
])

AC_ARG_ENABLE([sha512],
   AS_HELP_STRING([--disable-sha512], [disable ARMv8 SHA512 instructions]))
AS_IF([test "x$enable_sha512" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SHA512], [1], [disable ARMv8 SHA512 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHA512"
])

AC_ARG_ENABLE([sha3],
   AS_HELP_STRING([--disable-sha3], [disable ARMv8 SHA3 instructions]))
AS_IF([test "x$enable_sha3" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SHA3], [1], [disable ARMv8 SHA3 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHA3"
])

AC_ARG_ENABLE([sm3],
   AS_HELP_STRING([--disable-sm3], [disable ARMv8 SM3 instructions]))
AS_IF([test "x$enable_sm3" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SM3], [1], [disable ARMv8 SM3 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SM3"
])

AC_ARG_ENABLE([sm4],
   AS_HELP_STRING([--disable-sm4], [disable ARMv8 SM4 instructions]))
AS_IF([test "x$enable_sm4" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SM4], [1], [disable ARMv8 SM4 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SM4"
])

# PPC defines
AC_ARG_ENABLE([altivec],
   AS_HELP_STRING([--disable-altivec], [disable PPC Altivec instructions]))
AS_IF([test "x$enable_altivec" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_ALTIVEC], [1], [disable PPC Altivec instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ALTIVEC"
])

AC_ARG_ENABLE([aes],
   AS_HELP_STRING([--disable-aes], [disable PPC AES instructions]))
AS_IF([test "x$enable_aes" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_AES], [1], [disable PPC AES instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AES"
])

AC_ARG_ENABLE([vmull],
   AS_HELP_STRING([--disable-vmull], [disable PPC VMULL instructions]))
AS_IF([test "x$enable_vmull" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_VMULL], [1], [disable PPC VMULL instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_VMULL"
])

AC_ARG_ENABLE([sha256],
   AS_HELP_STRING([--disable-sha256], [disable PPC SHA256 instructions]))
AS_IF([test "x$enable_sha256" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SHA256], [1], [disable PPC SHA256 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHA256"
])

AC_ARG_ENABLE([sha512],
   AS_HELP_STRING([--disable-sha512], [disable PPC SHA512 instructions]))
AS_IF([test "x$enable_sha512" = "xno"], [
   AC_DEFINE([CRYPTOPP_DISABLE_SHA512], [1], [disable PPC SHA512 instructions])
   AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHA512"
])

#############################################################################
##   PART 4   PART 4   PART 4   PART 4   PART 4   PART 4   PART 4   PART 4
## ==========================================================================
## CPU feature tests, like SSE, NEON, ARMv8, PowerPC
#############################################################################

#############################################################################
## Die early on Android

if test "$IS_ANDROID_OS" != "0"; then

   AC_MSG_CHECKING([if ANDROID_NDK_ROOT is set])
   if test x"$ANDROID_NDK_ROOT" != "x"; then
      AC_MSG_RESULT([yes])
   else
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([ANDROID_NDK_ROOT is not set. Please set ANDROID_NDK_ROOT])
   fi
fi

#############################################################################
## Doxygen documentation
## This only provides a 'make html-doc' recipe in the Makefile.
## If Doxygen is available the user still must issue the command.

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test x"$DOXYGEN" = "x"; then
   AM_CONDITIONAL([CRYPTOPP_DOXYGEN_AVAILABLE], [false])
else
   AM_CONDITIONAL([CRYPTOPP_DOXYGEN_AVAILABLE], [true])
fi

#############################################################################
## Shared and static libraries

if true; then

   ## Cant use AC_ENABLE_STATIC and AC_ENABLE_SHARED. AIX Autotools is too old.
   AC_MSG_RESULT([checking if static libraries should be enabled... yes])

   ## http://stackoverflow.com/a/21455267/608639
   case $host_os in
      msys* | mingw* | cygwin* )
         AC_MSG_RESULT([checking if shared libraries should be enabled... no])
         enable_shared=no; enable_static=yes;
         ;;
      *)
         AC_MSG_RESULT([checking if shared libraries should be enabled... yes])
         enable_shared=yes; enable_static=yes;
         ;;
   esac
fi

#############################################################################
## Pthread library

if true; then

   ## Add to both CXXFLAGS and LDFLAGS if supported.
   ## https://stackoverflow.com/q/2127797/608639.

   SAVED_CXXFLAGS="$CXXFLAGS"
   SAVED_LDFLAGS="$LDFLAGS"

   CXXFLAGS="-pthread"
   LDFLAGS="-pthread"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   LDFLAGS="$SAVED_LDFLAGS"
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## Init priorities

## The tests below should be in two separate source file violating
## init order. One object file should perform the Foo or Bar static
## init, and the second source would have a static object using
## Foo::s_x or Bar::s_x. The problem is, we don't know how to do it.
## Also see https://stackoverflow.com/q/51662985/608639.

## We can only run one Init Priority test. Otherwise, running both tests
## cause 'CRYPTOPP_INIT_PRIORITY 250' and 'CRYPTOPP_INIT_PRIORITY 0' to be
## written, which results in "warning: CRYPTOPP_INIT_PRIORITY redefined".
## And of course, Autools does not allow us to undefine a macro.

## GCC-style
if true; then

   SAVED_CXXFLAGS="$CXXFLAGS"

   ## -Wall -Werror are GCC and friends specific. The problem is,
   ## Autotools lacks a cross-platform AC_TREAT_WARNINGS_AS_ERRORS.
   if test "$IS_MSC_COMPILER" = "0"; then
      THIS_CXXFLAGS="-Wall -Werror"
   fi

   CXXFLAGS="$THIS_CXXFLAGS"
   XXX_PROGRAM="#include <string>
      struct Foo {
         Foo(int x) : m_x(x) {}
         Foo(const Foo& o) : m_x(o.m_x) {}
         Foo& operator=(const Foo& o) {m_x=o.m_x; return *this;}
         static int s_x;
         int m_x;
      };
      int Foo::s_x = -1;
      static Foo unused __attribute__ ((init_priority (250))) = Foo::s_x;
      // This should be in a separate source file violating init order.
      // The separate file should have a main() and be linked first.
      // static Foo f = Foo::s_x;
      // bool success = f.m_x == -1;"

   AC_MSG_CHECKING([if $CXXNAME supports init_priority])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([$XXX_PROGRAM])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([HAVE_GCC_INIT_PRIORITY], [1])
      AC_DEFINE([CRYPTOPP_INIT_PRIORITY], [250], [Enable init_priority])
   else
      AC_DEFINE([CRYPTOPP_INIT_PRIORITY], [0], [Disable init_priority])
   fi

   CXXFLAGS="$SAVED_CXXFLAGS"
fi

## MSC-style
if false; then

   ## The problem is, Autotools lacks a cross-platform
   ## AC_TREAT_WARNINGS_AS_ERRORS. Clang, GCC, and friends will see
   ## "pragma init_seg" and issue an an unknown pragma warning.
   ## Adding -Wall or -Werror will cause MSC and friends to fail on
   ## the illegal flag.
   if test "$IS_MSC_COMPILER" = "1"; then
      THIS_CXXFLAGS="/WX"
   else
      THIS_CXXFLAGS="-Wall -Werror"
   fi

   CXXFLAGS="$THIS_CXXFLAGS"
   XXX_PROGRAM="#include <string>
      struct Bar {
         Bar(int x) : m_x(x) {}
         Bar(const Bar& o) : m_x(o.m_x) {}
         Bar& operator=(const Bar& o) {m_x=o.m_x; return *this;}
         static int s_x;
         int m_x;
      };
      #pragma init_seg(".CRT$XCU")
      int Bar::s_x = -1;
      static Bar unused __attribute__ ((init_priority (250))) = Bar::s_x;
      // This should be in a separate source file violating init order.
      // The separate file should have a main() and be linked first.
      // static Bar b = Bar::s_x;
      // bool success = b.m_x == -1;"

   AC_MSG_CHECKING([if $CXXNAME supports init_seg])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([$XXX_PROGRAM])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([HAVE_MSC_INIT_PRIORITY], [1])
      AC_DEFINE([CRYPTOPP_INIT_PRIORITY], [250], [Enable init_seg])
   else
      AC_DEFINE([CRYPTOPP_INIT_PRIORITY], [0], [Disable init_seg])
   fi

   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## Solaris, SunCC and missing optimizations

OPT_ANY=`echo $CXXFLAGS | $EGREP -c '\-O0|\-O1|\-O2|\-O3|\-O4|\-O5|\-xO0|\-xO1|\-xO2|\-xO3|\-xO4|\-xO5|\-Os|\-Og|\-Ofast'`

if test "$OPT_ANY" = "0"; then

   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-O2"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   else

      # SunCC
      CXXFLAGS="-xO2"
      AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
      XXX_COMPILE_IFELSE(
         [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
      fi
   fi

   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## Fix AR on iOS and OS X

## AR and ARFLAGS usage is currently broke under Autotools. Also see
## http://lists.gnu.org/archive/html/bug-autoconf/2017-11/msg00002.html

#if test "$IS_APPLE_OS" != "0"; then
#   AC_SUBST([AR], ["/usr/bin/libtool"])
#   AC_SUBST([ARFLAGS], ["-static -o"])
#   AC_SUBST([ac_ct_AR], ["/usr/bin/libtool"])
#fi

#############################################################################
## Dead code stripping for compilers. Linkers are towards the end of script.
## We have to use XXX_LINK_IFELSE because some compilers don't reject
## -fdata-sections or -ffunction-sections like they should.

if true; then

   ## Save FLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-fdata-sections"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   CXXFLAGS="-ffunction-sections"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   ## Restore FLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## Solaris, GCC and -Wa,--divide

if test "$IS_SUN_OS" != "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-Wa,--divide"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## Solaris, SunCC compiler options

if test "$IS_SUN_OS" != "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-template=no%extdef"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   # https://docs.oracle.com/cd/E18659_01/html/821-1383/bkamt.html
   CXXFLAGS="-xregs=no%appl"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## AIX and -qpic=large or -bbigtoc

if test "$IS_AIX" != "0"; then

   ## Save LDFLAGS
   SAVED_LDFLAGS="$LDFLAGS"

   # https://www.ibm.com/developerworks/rational/library/overview-toc-aix/index.html
   LDFLAGS="-bbigtoc"
   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   ## Restore LDFLAGS
   LDFLAGS="$SAVED_LDFLAGS"
fi

#############################################################################
## MacPorts GCC using Clang integrated assembler

## Modern GCC must use Clang as the assembler. The old Apple
## assembler cannot handle much more than SSE4 or so. We need
## to use -Wa,-q in subsequent tests, if available.

if test "$IS_APPLE_OS" != "0"; then
if test "$IS_GCC_COMPILER" != "0" && test "$IS_PORTS_COMPILER" != "0"; then

   HAVE_OPT=`echo $CXXFLAGS | $EGREP -i -c '\-Wa,-q'`
   if test "$HAVE_OPT" = "0"; then

      ## Save CXXFLAGS
      SAVED_CXXFLAGS="$CXXFLAGS"

      CXXFLAGS="-Wa,-q"
      AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
      XXX_COMPILE_IFELSE(
         [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      # We can add -Wa,-q to all files, if needed
      if test "$retval" = "0"; then
         # AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
         WAQ_FLAG="-Wa,-q"
      fi

      ## Restore CXXFLAGS
      CXXFLAGS="$SAVED_CXXFLAGS"
   fi
fi
fi

#############################################################################
## Clang integrated assembler and SunCC.

## Also see https://bugs.llvm.org/show_bug.cgi?id=39895.
if test "$IS_IA32" = "1" && test "x$enable_asm" != "xno"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-msse2 $WAQ_FLAG"
   AC_MSG_CHECKING([if $CXXNAME supports mixed ASM])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_asm_mixed.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" != "0"; then
      AC_DEFINE([CRYPTOPP_DISABLE_MIXED_ASM], [1], [Disable mixed asm])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_MIXED_ASM"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## IA-32, i386, i586, i686, x86_64, etc.

## This block handles GCC and friends, including -Wa,-q if needed.
## i86pc and SunCC is handled after this block.
if test "$IS_IA32" = "1" && test "$IS_GNUC_COMPILER" = "1"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   ##### SSE2 #####

   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-msse2 $WAQ_FLAG"
   else
      CXXFLAGS="-msse2"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sse2.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SSE2_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CHACHA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_DONNA_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_SSE2_AVAILABLE], [1], [Enable SSE2])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ASM], [1], [Disable SSE2])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ASM"
   fi

   ##### SSE3 #####

   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-msse3 $WAQ_FLAG"
   else
      CXXFLAGS="-msse3"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sse3.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_DEFINE([CRYPTOPP_SSE3_AVAILABLE], [1], [Enable SSE3])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SSE3], [1], [Disable SSE3])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE3"
   fi

   ##### SSSE3 #####

   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-mssse3 $WAQ_FLAG"
   else
      CXXFLAGS="-mssse3"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_ssse3.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SSSE3_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_ARIA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CHAM_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_KECCAK_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_LEA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SIMON128_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SPECK128_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_SSSE3_AVAILABLE], [1], [Enable SSSE3])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SSSE3], [1], [Disable SSSE3])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSSE3"
   fi

   ##### SSE4.1 #####

   ## Old BSD linkers don't support SSE4 or above.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-msse4.1 $WAQ_FLAG"
   else
      CXXFLAGS="-msse4.1"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sse41.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SSE41_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_BLAKE2S_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_BLAKE2B_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_SSE41_AVAILABLE], [1], [Enable SSE4.1])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SSE4], [1], [Disable SSE4.1])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE4"
   fi

   ##### SSE4.2 #####

   ## Old BSD linkers don't support it.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-msse4.2 $WAQ_FLAG"
   else
      CXXFLAGS="-msse4.2"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sse42.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SSE42_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CRC_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_SSE42_AVAILABLE], [1], [Enable SSE4.2])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SSE4], [1], [Disable SSE4.2])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE4"
   fi

   ##### PCLMUL #####

   ## Some old compilers, assemblers and linkers don't support it.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-mpclmul $WAQ_FLAG"
   else
      CXXFLAGS="-mpclmul"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_clmul.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   # if -mpclmul fails then it is disabled in the next test
   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_GF2N_FLAG], [$CXXFLAGS])
   fi

   ## Some old compilers, assemblers and linkers don't support it.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-mssse3 -mpclmul $WAQ_FLAG"
   else
      CXXFLAGS="-mssse3 -mpclmul"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_clmul.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_GCM_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_CLMUL_AVAILABLE], [1], [Enable CLMUL])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_CLMUL], [1], [Disable CLMUL])
   fi

   ##### SSSE3+AES #####

   ## Some old compilers, assemblers and linkers don't support it.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-mssse3 -maes $WAQ_FLAG"
   else
      CXXFLAGS="-mssse3 -maes"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_aes.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   # if -maes fails then it is disabled in the next test
   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SM4_FLAG], [$CXXFLAGS])
   fi

   ##### SSE4.1+AES #####

   ## Some old compilers, assemblers and linkers don't support it.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-msse4.1 -maes $WAQ_FLAG"
   else
      CXXFLAGS="-msse4.1 -maes"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_aes.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_AES_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_AESNI_AVAILABLE], [1], [Enable AESNI])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_AESNI], [1], [Disable AESNI])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AESNI"
   fi

   ##### AVX #####

   ## Some old compilers, assemblers and linkers don't support it.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-mavx $WAQ_FLAG"
   else
      CXXFLAGS="-mavx"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_avx.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_CHACHA_AVX_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_AVX_AVAILABLE], [1], [Enable AVX])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_AVX], [1], [Disable AVX])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AVX"
   fi

   ##### AVX2 #####

   ## Some old compilers, assemblers and linkers don't support it.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-mavx2 $WAQ_FLAG"
   else
      CXXFLAGS="-mavx2"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_avx2.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_CHACHA_AVX2_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_AVX2_AVAILABLE], [1], [Enable AVX2])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_AVX2], [1], [Disable AVX2])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AVX2"
   fi

   ##### SHA #####

   ## Some old compilers, assemblers and linkers don't support it.
   if test x"$WAQ_FLAG" != "x"; then
      CXXFLAGS="-msse4.2 -msha $WAQ_FLAG"
   else
      CXXFLAGS="-msse4.2 -msha"
   fi

   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sha.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SHA_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_SHANI_AVAILABLE], [1], [Enable SHANI])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SHANI], [1], [Disable SHANI])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHANI"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

## This block handles i86pc and SunCC. GCC and friends were handled before this block
if test "$IS_IA32" != "0" && test "$IS_SUN_COMPILER" != "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   ##### SSE2 #####

   CXXFLAGS="-xarch=sse2"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sse2.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   ## GCM AES and SHA should be overwritten with CLMUL, AES and SHA if available.
   ## If not available then need -xarch=sse2 due to SSE2, GCM_Xor16_SSE2, etc.
   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__SSE2__=1"
      AC_SUBST([CRYPTOPP_SSE2_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_CHACHA_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_GCM_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_AES_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_SHA_FLAG], [$CXXFLAGSX])
      AC_DEFINE([CRYPTOPP_SSE2_AVAILABLE], [1], [Enable SSE2])
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ASM], [1], [Disable SSE2])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ASM"
   fi

   ##### SSE3 #####

   ## Nothing pivots around SSE3 specifically, but we do use
   ## insns from the ISA. Sun requires it as a linker flag.
   CXXFLAGS="-xarch=sse3"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sse3.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__SSE3__=1"
      AC_DEFINE([CRYPTOPP_SSE3_AVAILABLE], [1], [Enable SSE3])
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SSE3], [1], [Disable SSE3])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE3"
   fi

   ##### SSSE3 #####

   CXXFLAGS="-xarch=ssse3"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_ssse3.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__SSSE3__=1"
      AC_SUBST([CRYPTOPP_SSSE3_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_ARIA_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_CHACHA_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_CHAM_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_KECCAK_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_LEA_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_SIMON128_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_SPECK128_FLAG], [$CXXFLAGSX])
      AC_DEFINE([CRYPTOPP_SSSE3_AVAILABLE], [1], [Enable SSSE3])
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SSSE3], [1], [Disable SSSE3])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSSE3"
   fi

   ##### SSE4.1 #####

   CXXFLAGS="-xarch=sse4_1"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sse41.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__SSE4_1__=1"
      AC_SUBST([CRYPTOPP_SSE41_FLAG], [$CXXFLAGSX])
      AC_DEFINE([CRYPTOPP_SSE41_AVAILABLE], [1], [Enable SSE4.1])
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SSE4], [1], [Disable SSE4.1])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE4"
   fi

   ##### SSE4.2 #####

   CXXFLAGS="-xarch=sse4_2"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sse42.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__SSE4_2__=1"
      AC_SUBST([CRYPTOPP_SSE42_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_CRC_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_BLAKE2S_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_BLAKE2B_FLAG], [$CXXFLAGSX])
      AC_DEFINE([CRYPTOPP_SSE42_AVAILABLE], [1], [Enable SSE4.2])
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SSE4], [1], [Disable SSE4.2])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SSE4"
   fi

   ##### CLMUL #####

   CXXFLAGS="-xarch=aes"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_clmul.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__CLMUL__=1"
      AC_SUBST([CRYPTOPP_GCM_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_GF2N_FLAG], [$CXXFLAGSX])
      AC_DEFINE([CRYPTOPP_CLMUL_AVAILABLE], [1], "Enable CLMUL")
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_CLMUL], [1], "Disable CLMUL")
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_CLMUL"
   fi

   ##### AES #####

   CXXFLAGS="-xarch=aes"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_aes.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__AES__=1"
      AC_SUBST([CRYPTOPP_AES_FLAG], [$CXXFLAGSX])
      AC_SUBST([CRYPTOPP_SM4_FLAG], [$CXXFLAGSX])
      AC_DEFINE([CRYPTOPP_AESNI_AVAILABLE], [1], "Enable AESNI")
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_AESNI], [1], "Disable AESNI")
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AESNI"
   fi

   ##### AVX #####

   CXXFLAGS="-xarch=avx"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_avx.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__AVX__=1"
      AC_DEFINE([CRYPTOPP_AVX_AVAILABLE], [1], [Enable AVX])
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_AVX], [1], [Disable AVX])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AVX"
   fi

   ##### AVX2 #####

   CXXFLAGS="-xarch=avx2"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_avx2.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__AVX2__=1"
      AC_SUBST([CRYPTOPP_CHACHA_AVX2_FLAG], [$CXXFLAGSX])
      AC_DEFINE([CRYPTOPP_AVX2_AVAILABLE], [1], [Enable AVX2])
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_AVX2], [1], [Disable AVX2])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_AVX2"
   fi

   ##### RDRAND #####

   ## https://docs.oracle.com/cd/E77782_01/html/E77792/gqexw.html
   CXXFLAGS="-xarch=avx_i"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_rdrand.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   ## It is not clear if we can avoid the linker option for Solaris
   ## Also see https://stackoverflow.com/q/51678753/608639
   #if test "$retval" = "0"; then
   #   CXXFLAGSX="$CXXFLAGS -D__RDRND__=1"
   #   AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   #fi

   ##### RDSEED #####

   ## https://docs.oracle.com/cd/E60778_01/html/E60745/bjapr.html
   CXXFLAGS="-xarch=avx2_i"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_rdseed.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   ## It is not clear if we can avoid the linker option for Solaris
   ## Also see https://stackoverflow.com/q/51678753/608639
   #if test "$retval" = "0"; then
   #   CXXFLAGSX="$CXXFLAGS -D__RDSEED__=1"
   #   AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   #fi

   ##### SHA #####

   ##CXXFLAGS="-xarch=avx2_i"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_x86_sha.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      CXXFLAGSX="$CXXFLAGS -D__SHA__=1"
      AC_SUBST([CRYPTOPP_SHA_FLAG], [$CXXFLAGSX])
      AC_DEFINE([CRYPTOPP_SHA_AVAILABLE], [1], "Enable SHANI")
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   else
      AC_DEFINE([CRYPTOPP_DISABLE_SHANI], [1], "Disable SHANI")
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_SHANI"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## ARM A-32, including NEON, soft floats and hard floats

if test "$IS_ARM32" != "0" || test "$IS_ARMV8" != "0"; then

   # https://gcc.gnu.org/onlinedocs/gcc/ARM-Options.html

   AC_CHECK_HEADERS(arm_neon.h,
      [AC_DEFINE([CRYPTOPP_ARM_NEON_HEADER], [1], [arm_neon.h is available])],
      [AC_DEFINE([CRYPTOPP_ARM_NEON_HEADER], [0], [arm_neon.h is not available])]
   )
fi

if test "$IS_ARM32" != "0" && test "$IS_ANDROID_OS" = "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-march=armv7-a"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_cxx.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   CXXFLAGS="-march=armv7-a -mfpu=neon"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_neon.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_AES_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_ARIA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_BLAKE2S_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_BLAKE2B_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CHACHA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CHAM_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CRC_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_GCM_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_LEA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_NEON_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SHA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SM4_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SIMON128_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SPECK64_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SPECK128_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_NEON_AVAILABLE], [1], [Enable NEON])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ASM], [1], [Disbale NEON])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ASM"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## Android and -mfloat-abi=softfp

if test "$IS_ARM32" != "0" && test "$IS_ANDROID_OS" != "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=neon"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_neon.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_AES_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_ARIA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_BLAKE2S_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_BLAKE2B_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CHACHA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CHAM_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CRC_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_GCM_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_LEA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_NEON_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SHA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SM4_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SIMON128_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SPECK128_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_NEON_AVAILABLE], [1], [Enable NEON])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ASM], [1], [Disbale NEON])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ASM"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"

fi

#############################################################################
## ARM A-32 and aes_armv4.S

# Used by Makefile.am to compile aes_armv4.S and sha1_armv4.S.
# Only valid on Linux because ASM is written for GNU AS (GAS).
if test "$IS_ARM32" != "0" && test "$IS_LINUX_OS" != "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-march=armv7-a -Wa,--noexecstack"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_cxx.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CONDITIONAL([CRYPTOGAMS_ARM_SOURCES], [true])
      AC_SUBST([CRYPTOPGAMS_ARMV4_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPGAMS_ARMV4_THUMB_FLAG], [$CXXFLAGS])
   fi

   # For Clang, aes_armv4.S needs -mthumb
   if test "$IS_LLVM_COMPILER" != "0"; then

      CXXFLAGS="-march=armv7-a -mthumb -Wa,--noexecstack"
      AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
      XXX_COMPILE_IFELSE(
         [AC_LANG_SOURCE([`cat TestPrograms/test_cxx.cxx`])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AC_SUBST([CRYPTOPGAMS_ARMV4_THUMB_FLAG], [$CXXFLAGS])
      fi
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
else
   # Required for other platforms
   AM_CONDITIONAL([CRYPTOGAMS_ARM_SOURCES], [false])
fi

#############################################################################
## ARM Aarch32 and Aarch64

if test "$IS_ARMV8" != "0"; then

   # https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html

   AC_CHECK_HEADERS(arm_acle.h,
      [AC_DEFINE([CRYPTOPP_ARM_ACLE_HEADER], [1], [arm_acle.h is available])],
      [AC_DEFINE([CRYPTOPP_ARM_ACLE_HEADER], [0], [arm_acle.h is not available])]
   )

fi

if test "$IS_ARMV8" != "0"; then

   # https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-march=armv8-a"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_asimd.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_ARIA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CHACHA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_CHAM_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_BLAKE2S_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_BLAKE2B_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_LEA_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_NEON_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SIMON128_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_SPECK128_FLAG], [$CXXFLAGS])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ASM], [1], [Enable NEON])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ASM"
   fi

   CXXFLAGS="-mfix-cortex-a53-835769"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_cxx.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   CXXFLAGS="-mfix-cortex-a53-843419"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_cxx.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $CXXFLAGS"
   fi

   CXXFLAGS="-march=armv8-a+crc"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_crc.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_CRC_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_CRC32_AVAILABLE], [1], [Enable ARMv8 CRC32])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_CRC32], [1], [Disable ARMv8 CRC32])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_CRC32"
   fi

   CXXFLAGS="-march=armv8-a+crypto"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS (AES)])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_aes.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_AES_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_AES_AVAILABLE], [1], [Enable ARMv8 AES])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ARM_AES], [1], [Disable ARMv8 AES])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_AES"
   fi

   CXXFLAGS="-march=armv8-a+crypto"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS (PMULL)])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_pmull.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_GCM_FLAG], [$CXXFLAGS])
      AC_SUBST([CRYPTOPP_GF2N_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_PMULL_AVAILABLE], [1], [Enable ARMv8 PMULL])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ARM_PMULL], [1], [Disable ARMv8 PMULL])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_PMULL"
   fi

   CXXFLAGS="-march=armv8-a+crypto"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS (SHA1)])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_sha1.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SHA_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_SHA1_AVAILABLE], [1], [Enable ARMv8 SHA1])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ARM_SHA], [1], [Disable ARMv8 SHA1])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_SHA"
   fi

   CXXFLAGS="-march=armv8-a+crypto"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS (SHA256)])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_sha256.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SHA_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_SHA256_AVAILABLE], [256], [Enable ARMv8 SHA256])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ARM_SHA], [256], [Disable ARMv8 SHA256])
      AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_SHA"
   fi

   CXXFLAGS="-march=armv8.3-a+sha3"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS (SHA512)])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_sha3.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SHA512_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_SHA512_AVAILABLE], [1], [Enable ARMv8 SHA512])
   #else
      #AC_DEFINE([CRYPTOPP_DISABLE_ARM_SHA512], [1], [Disable ARMv8 SHA512])
      #AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_SHA512"
   fi

   CXXFLAGS="-march=armv8.3-a+sha3"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS (SHA3)])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_sha3.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SHA3_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_SHA3_AVAILABLE], [1], [Enable ARMv8 SHA3])
   #else
      #AC_DEFINE([CRYPTOPP_DISABLE_ARM_SHA3], [1], [Disable ARMv8 SHA3])
      #AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_SHA3"
   fi

   CXXFLAGS="-march=armv8.3-a+sm4"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS (SM3)])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_sm3.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SM3_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_SM3_AVAILABLE], [1], [Enable ARMv8 SM3])
   #else
      #AC_DEFINE([CRYPTOPP_DISABLE_ARM_SM3], [1], [Disable ARMv8 SM3])
      #AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_SM3"
   fi

   CXXFLAGS="-march=armv8.3-a+sm4"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS (SM4)])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_arm_sm4.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_SM4_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_SM4_AVAILABLE], [1], [Enable ARMv8 SM4])
   #else
      #AC_DEFINE([CRYPTOPP_DISABLE_ARM_SM4], [1], [Disable ARMv8 SM4])
      #AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_SM4"
   fi

   CXXFLAGS="-march=armv8.3-a+rng"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_SUBST([CRYPTOPP_ARM_RNG_FLAG], [$CXXFLAGS])
      AC_DEFINE([CRYPTOPP_ARM_RNG_AVAILABLE], [1], [Enable ARMv8 RNG])
   #else
      #AC_DEFINE([CRYPTOPP_DISABLE_ARM_RNG], [1], [Disable ARMv8 RNG])
      #AM_CXXFLAGS="$AM_CXXFLAGS -DCRYPTOPP_DISABLE_ARM_RNG"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## IBC XL C/C++ compiler, applies to IA-32 and PPC

if test "$IS_XLC_COMPILER" != "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   ## -qrtti must be present for some xlC compilers
   HAS_RTTI=`echo $CPPFLAGS $CXXFLAGS | $EGREP -i -c '\-qrtti'`
   if test "$HAS_RTTI" = "0"; then

      CXXFLAGS="-qrtti"
      AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
      XXX_COMPILE_IFELSE(
         [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
      fi
   fi

   ## -qpic must be present for some xlC compilers
   HAS_PIC=`echo $CPPFLAGS $CXXFLAGS | $GREP -i -c '\-qpic'`
   if test "$HAS_PIC" = "0"; then

      CXXFLAGS="-qpic"
      AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
      XXX_COMPILE_IFELSE(
         [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
      fi
   fi

   # Promotes optimizations
   CXXFLAGS="-qmaxmem=-1"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   # Promotes optimizations
   CXXFLAGS="-qthreaded"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   if test "$IS_DEBUG" != "0"; then

      # Finds alias violations
      CXXFLAGS="-qalias=noansi"
      AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
      XXX_COMPILE_IFELSE(
         [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
      fi
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
# More stupid LLVM games. Clang pretending to be three different compilers.
# https://lists.tetaneutral.net/pipermail/cfarm-users/2018-July/000331.html

if test "$IS_XLC_COMPILER" != "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-qxlcompatmacros"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"

fi

#############################################################################
## PowerPC and PowerPC-64

if test "$IS_PPC" != "0"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   # XLC requires -qaltivec in addition to Arch or CPU option
   if test "$IS_XLC_COMPILER" = "1"; then
      ALTIVEC_FLAG="-qarch=auto -qaltivec"
      POWER7_VSX_FLAG="-qarch=pwr7 -qvsx -qaltivec"
      POWER7_PWR_FLAG="-qarch=pwr7 -qaltivec"
      POWER8_FLAG="-qarch=pwr8 -qaltivec"
      POWER9_FLAG="-qarch=pwr9 -qaltivec"
   else
      ALTIVEC_FLAG="-maltivec"
      POWER7_VSX_FLAG="-mcpu=power7 -mvsx"
      POWER7_PWR_FLAG="-mcpu=power7"
      POWER8_FLAG="-mcpu=power8"
      POWER9_FLAG="-mcpu=power9"
   fi

   ##### Altivec #####

   CXXFLAGS="$ALTIVEC_FLAG"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_ppc_altivec.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   # Assume success, unset on failure
   if test "$retval" != "0"; then
       AC_SUBST([ALTIVEC_FLAG],[""])
   fi

   ##### POWER7 #####

   CXXFLAGS="$POWER7_VSX_FLAG"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_ppc_power7.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   # Figure out what is needed for POWER7
   if test "$retval" = "0"; then
     AC_SUBST([POWER7_FLAG],[$POWER7_VSX_FLAG])
   else
     CXXFLAGS="$POWER7_PWR_FLAG"
     AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
     XXX_LINK_IFELSE(
        [AC_LANG_SOURCE([`cat TestPrograms/test_ppc_power7.cxx`])],
        [AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])]
     )

     if test "$retval" = "0"; then
       AC_SUBST([POWER7_FLAG],[$POWER7_PWR_FLAG])
     else
       AC_SUBST([POWER7_FLAG],[""])
     fi  # POWER7_PWR_FLAG
   fi  # POWER7_VSX_FLAG

   ##### POWER8 #####

   CXXFLAGS="$POWER8_FLAG"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_ppc_power8.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" != "0"; then
       AC_SUBST([POWER8_FLAG],[""])
   fi

   ##### POWER9 #####

   CXXFLAGS="$POWER9_FLAG"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_ppc_power9.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" != "0"; then
       AC_SUBST([POWER9_FLAG],[""])
   fi

   # POWER9 and DARN random number generator. Disabled due to
   # https://github.com/weidai11/cryptopp/issues/986
   #if test x"$POWER9_FLAG" != "x"; then
   #   AC_SUBST([CRYPTOPP_POWER9_FLAG], [$POWER9_FLAG])
   #   AC_SUBST([CRYPTOPP_DARN_FLAG], [$POWER9_FLAG])
   #else
   #   AC_DEFINE([CRYPTOPP_DISABLE_POWER9], [1], [Disable POWER9])
   #fi

   ##### Fixups #####

   ## POWER8 and in-core crypto.
   if test x"$POWER8_FLAG" != "x"; then
      AC_SUBST([CRYPTOPP_POWER8_FLAG], [$POWER8_FLAG])
      AC_SUBST([CRYPTOPP_BLAKE2B_FLAG], [$POWER8_FLAG])
      AC_SUBST([CRYPTOPP_CRC_FLAG], [$POWER8_FLAG])
      AC_SUBST([CRYPTOPP_AES_FLAG], [$POWER8_FLAG])
      AC_SUBST([CRYPTOPP_GCM_FLAG], [$POWER8_FLAG])
      AC_SUBST([CRYPTOPP_GF2N_FLAG], [$POWER8_FLAG])
      AC_SUBST([CRYPTOPP_SHA_FLAG], [$POWER8_FLAG])
      AC_SUBST([CRYPTOPP_SIMON128_FLAG], [$POWER8_FLAG])
      AC_SUBST([CRYPTOPP_SPECK128_FLAG], [$POWER8_FLAG])
      AC_DEFINE([CRYPTOPP_POWER8_CRC_AVAILABLE], [1], [Enable POWER8 CRC])
      AC_DEFINE([CRYPTOPP_POWER8_AES_AVAILABLE], [1], [Enable POWER8 AES])
      AC_DEFINE([CRYPTOPP_POWER8_PMULL_AVAILABLE], [1], [Enable POWER8 PMULL])
      AC_DEFINE([CRYPTOPP_POWER8_SHA_AVAILABLE], [1], [Enable POWER8 SHA])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_POWER8], [1], [Disable POWER8])
   fi

   # POWER7 and VSX
   if test x"$POWER7_FLAG" != "x"; then
      AC_SUBST([CRYPTOPP_POWER7_FLAG], [$POWER7_FLAG])
      AC_DEFINE([CRYPTOPP_POWER7_AVAILABLE], [1], [Enable POWER7])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_POWER7], [1], [Disable POWER7])
   fi

   # Plain old Altivec
   if test x"$ALTIVEC_FLAG" != "x"; then
      AC_SUBST([CRYPTOPP_ALTIVEC_FLAG], [$ALTIVEC_FLAG])
      AC_SUBST([CRYPTOPP_BLAKE2S_FLAG], [$ALTIVEC_FLAG])
      AC_SUBST([CRYPTOPP_CHACHA_FLAG], [$ALTIVEC_FLAG])
      AC_DEFINE([CRYPTOPP_ALTIVEC_AVAILABLE], [1], [Enable Altivec])
   else
      AC_DEFINE([CRYPTOPP_DISABLE_ALTIVEC], [1], [Disable Altivec])
   fi

   # Drop to Altivec if higher Power is not available
   if test x"$ALTIVEC_FLAG" != "x"; then
      if test x"$CRYPTOPP_GCM_FLAG" = "x"; then
         AC_SUBST([CRYPTOPP_GCM_FLAG], [$ALTIVEC_FLAG])
      fi
   fi

   # Now set a disable flag
   if test x"$ALTIVEC_FLAG" = "x"; then
      AM_CXXFLAGS="-DCRYPTOPP_DISABLE_ALTIVEC $AM_CXXFLAGS"
   elif test x"$POWER7_FLAG" = "x"; then
      AM_CXXFLAGS="-DCRYPTOPP_DISABLE_POWER7 $AM_CXXFLAGS"
   elif test x"$POWER8_FLAG" = "x"; then
      AM_CXXFLAGS="-DCRYPTOPP_DISABLE_POWER8 $AM_CXXFLAGS"
   elif test x"$POWER9_FLAG" = "x"; then
      AM_CXXFLAGS="-DCRYPTOPP_DISABLE_POWER9 $AM_CXXFLAGS"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
##   PART 5   PART 5   PART 5   PART 5   PART 5   PART 5   PART 5   PART 5
## ==========================================================================
## C++ feature tests, like C++11, alignof and alignas
#############################################################################

#############################################################################
## Newlib and _XOPEN_SOURCE

if true; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   ## This test may need to be changed if the Newlib folks provide directions.
   ## Also see http://www.sourceware.org/ml/newlib/2017/msg01095.html.

   AC_MSG_CHECKING([whether runtime library is Newlib])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_newlib.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then

      CXXFLAGS="-D_XOPEN_SOURCE=600"
      AC_MSG_CHECKING([if runtime library supports $CXXFLAGS])
      XXX_COMPILE_IFELSE(
         [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AC_DEFINE([_XOPEN_SOURCE], [600], [Enable _XOPEN_SOURCE])
         AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
      fi
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## libstdc++ and _GLIBCXX_DEBUG
if test "$IS_DEBUG" = "1"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   AC_MSG_CHECKING([whether runtime library is libstdc++])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_glibc.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then

      CXXFLAGS="-D_GLIBCXX_DEBUG"
      AC_MSG_CHECKING([if runtime library supports $CXXFLAGS])
      XXX_COMPILE_IFELSE(
         [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
      fi
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## Now test C++ features

#AH_HEADER=
#AC_CONFIG_COMMANDS_PRE(
#   [m4_ifndef([AH_HEADER], [AC_CONFIG_HEADERS([config_cxx.h])])
#])

#############################################################################

if true; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   AC_MSG_CHECKING([whether runtime library is C++11])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_cxx11.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_DEFINE([CRYPTOPP_CXX11], [1], [C++11 is available])
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

if true; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   AC_MSG_CHECKING([whether runtime library is C++17])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([`cat TestPrograms/test_cxx17.cxx`])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AC_DEFINE([CRYPTOPP_CXX17], [1], [C++17 is available])
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
##   PART 6   PART 6   PART 6   PART 6   PART 6   PART 6   PART 6   PART 6
## ==========================================================================
## Misc feature and options
#############################################################################

#############################################################################
## Add -pipe for non-IoT gadgets to speedup compiles. We use 2GB RAM as
## threshold. Anything with less than 2GB will not use -pipe.

# Unix, Linux and family
MEM_SIZE=`$GREP "MemTotal" /proc/meminfo 2>/dev/null | $AWK '{print int($2/1024/1024); exit;}'`

# Darwin
if test "$IS_APPLE_OS" != "0"; then
   MEM_SIZE=`sysctl -a 2>&1 | $GREP "hw.memsize" 2>/dev/null | $AWK '{print int($3/1024/1024); exit;}'`
fi

# Solaris
if test "$IS_SUN_OS" != "0"; then
   MEM_SIZE=`prtconf 2>/dev/null | $GREP "Memory" | $AWK '{print int($3/1024); exit;}'`
fi

# AIX
if test "$IS_AIX_OS" != "0"; then
   MEM_SIZE=`prtconf -m 2>/dev/null | $GREP "MB" | $AWK '{print int($3/1024); exit;}'`
fi

if test "$MEM_SIZE" -ge "2"; then

   ## Save CXXFLAGS
   SAVED_CXXFLAGS="$CXXFLAGS"

   CXXFLAGS="-pipe"
   AC_MSG_CHECKING([if $CXXNAME supports $CXXFLAGS])
   XXX_COMPILE_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
   fi

   ## Restore CXXFLAGS
   CXXFLAGS="$SAVED_CXXFLAGS"
fi

#############################################################################
## Dead code stripping for linkers. Compilers are towards the head of script.

if true; then

   ## Save FLAGS
   SAVED_LDFLAGS="$LDFLAGS"

   # OS X uses -Wl,-dead_strip
   LDFLAGS="-Wl,-dead_strip"
   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   # Linux uses --gc-sections
   LDFLAGS="-Wl,--gc-sections"
   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   # Solaris uses -z discard-unused
   # https://docs.oracle.com/cd/E26502_01/html/E26507/glmth.html
   LDFLAGS="-Wl,-z,discard-unused=sections"
   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   # Solaris uses -z discard-unused
   # https://docs.oracle.com/cd/E26502_01/html/E26507/glmth.html
   LDFLAGS="-Wl,-z,discard-unused=dependencies"
   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   ## Restore FLAGS
   LDFLAGS="$SAVED_LDFLAGS"
fi

#############################################################################
## Solaris, map file and libraries

if test "$IS_SUN_OS" != "0"; then

   ## Save LDFLAGS
   SAVED_LDFLAGS="$LDFLAGS"

   ## IA-32 gets a Mapfile for capabilities. Due to libtool stripping
   ## -M arguments, we need to escape it with -XCClinker in AM_LDFLAGS.
   if test "$IS_IA32" != "0"; then
      LDFLAGS="-Mcryptopp.mapfile"
      AC_MSG_CHECKING([if $LDNAME supports -Mcryptopp.mapfile])
      XXX_LINK_IFELSE(
        [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AM_LDFLAGS="$AM_LDFLAGS -XCClinker -Mcryptopp.mapfile"
      fi
   fi

   ## Restore LDFLAGS
   LDFLAGS="$SAVED_LDFLAGS"
fi

#############################################################################
## Add --build-id to linker for debug symbols.
## https://sourceware.org/gdb/onlinedocs/gdb/Separate-Debug-Files.html

if true; then

   SAVED_LDFLAGS="$LDFLAGS"
   LDFLAGS="-Wl,--build-id"

   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   LDFLAGS="$SAVED_LDFLAGS"
fi

#############################################################################
## Demangle output names if possible for friendlier err messages

if true; then

   SAVED_LDFLAGS="$LDFLAGS"
   LDFLAGS="-Wl,--demangle"

   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   LDFLAGS="$SAVED_LDFLAGS"
fi

#############################################################################
## MinGW libraries
## Automake does not follow Make conventions and use AM_LDLIBS. Instead
## it stuffs libraries in the flags which messes up pkg-config *.pc files.

if test "$IS_MINGW_OS" = "1"; then

   SAVED_LDFLAGS="$LDFLAGS"

   LDFLAGS="-lws2_32"
   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   ## Restore LDFLAGS
   LDFLAGS="$SAVED_LDFLAGS"
fi

#############################################################################
## OpenMP libraries
## Automake does not follow Make conventions and use AM_LDLIBS. Instead
## it stuffs libraries in the flags which messes up pkg-config *.pc files.

if true; then

   COUNT1=`echo $CXXFLAGS | $EGREP -c '\-fopenmp'`
   COUNT2=`echo $LDFLAGS | $EGREP -c '\-lgomp'`
   if test "$COUNT1" != "0" && test "$COUNT2" = "0"; then

      SAVED_LDFLAGS="$LDFLAGS"

      LDFLAGS="-lgomp"
      AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
      XXX_LINK_IFELSE(
         [AC_LANG_SOURCE([[int main() { return 0; }]])],
         [AC_MSG_RESULT([yes])],
         [AC_MSG_RESULT([no])]
      )

      if test "$retval" = "0"; then
         AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
      fi

      LDFLAGS="$SAVED_LDFLAGS"
   fi
fi

#############################################################################
## Add -Wl,--exclude-libs,ALL for GNU linker to stop us from re-exporting
## symbols in other libraries. This keeps our ELF section header smaller.

if test "$enable_shared" = "yes"; then

   SAVED_LDFLAGS="$LDFLAGS"
   LDFLAGS="-Wl,--exclude-libs,ALL"

   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   LDFLAGS="$SAVED_LDFLAGS"
fi

#############################################################################
## Add -Wl,--as-needed for linker to avoid linking unneeded libraries.

if true; then

   SAVED_LDFLAGS="$LDFLAGS"
   LDFLAGS="-Wl,--as-needed"

   AC_MSG_CHECKING([if $LDNAME supports $LDFLAGS])
   XXX_LINK_IFELSE(
      [AC_LANG_SOURCE([[int main() { return 0; }]])],
      [AC_MSG_RESULT([yes])],
      [AC_MSG_RESULT([no])]
   )

   if test "$retval" = "0"; then
      AM_LDFLAGS="$AM_LDFLAGS $LDFLAGS"
   fi

   LDFLAGS="$SAVED_LDFLAGS"
fi

#############################################################################
## Android cpu-features library

if test "$IS_ANDROID_OS" != "0"; then

   # ac_srcdir does not work. Autotools blows...

   THIS_FILE="$ANDROID_NDK_ROOT/sources/android/cpufeatures/cpu-features.h"
   AC_CHECK_FILE([$THIS_FILE],
      [cp "$THIS_FILE" "./"]
   )

   THIS_FILE="$ANDROID_NDK_ROOT/sources/android/cpufeatures/cpu-features.c"
   AC_CHECK_FILE([$THIS_FILE],
      [cp "$THIS_FILE" "./"]
   )

   # And set the flag so Makefile adds the recipes
   THIS_FILE="./cpu-features.c"
   AC_CHECK_FILE([$THIS_FILE],
      [AM_CONDITIONAL([ANDROID_CPU_FEATURES], [true])],
      [AM_CONDITIONAL([ANDROID_CPU_FEATURES], [false])]
   )
else
   AM_CONDITIONAL([ANDROID_CPU_FEATURES], [false])
fi

#############################################################################
## Miscellaneous items

## Fix CCAS and CCASFLAGS
## Also see https://stackoverflow.com/q/51575580/608639
AC_SUBST([CCAS], [$CXX])
AC_SUBST([CCASFLAGS], ["$AM_CXXFLAGS $CXXFLAGS"])

## Without this 'make' uses our GNUmakefile, and not Makefile
AC_CONFIG_COMMANDS_PRE([rm -f GNUmakefile 2>/dev/null])
AC_CONFIG_COMMANDS_PRE([rm -f GNUmakefile-cross 2>/dev/null])

## Write the Automake flags we set
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

## Write the output artifacts
AC_OUTPUT

## Automake bug workaround. If libcryptopp_la_SOURCES is an empty list, Automake assumes
## C source files and drive link through the C compiler. We provide the empty adhoc.cpp
## to get things back on course, so adhoc.cpp must always be copied.

AC_CONFIG_COMMANDS_POST([cp adhoc.cpp.proto adhoc.cpp])

#############################################################################
## Print a summary of information

echo ""
echo "***********************************************************************"

echo "Auto-configuration complete. A summary of options are below. If"
echo "something looks wrong then please modify config.h and please report"
echo "it at http://github.com/noloader/cryptopp-autotools."
echo ""

echo "   Build triplet: $build"
echo " Compiler target: $COMPILER_TARGET"
echo "Compiler version: $COMPILER_VERSION"
echo ""

echo "Static library: $enable_static"
echo "Shared library: $enable_shared"
echo ""

if test "$IS_ARM32" != "0" || test "$IS_ARMV8" != "0"; then
  if test x"$CRYPTOPP_ARM_NEON_HEADER" != "x"; then
    echo "CRYPTOPP_ARM_NEON_HEADER: $CRYPTOPP_ARM_NEON_HEADER"
  fi
  if test x"$CRYPTOPP_ARM_ACLE_HEADER" != "x"; then
    echo "CRYPTOPP_ARM_ACLE_HEADER: $CRYPTOPP_ARM_ACLE_HEADER"
  fi
  echo ""
fi

if test x"$CRYPTOPP_SSE2_FLAG" != "x"; then
   echo "CRYPTOPP_SSE2_FLAG: $CRYPTOPP_SSE2_FLAG"
fi
if test x"$CRYPTOPP_NEON_FLAG" != "x"; then
   echo "CRYPTOPP_NEON_FLAG: $CRYPTOPP_NEON_FLAG"
fi
if test x"$CRYPTOPP_ALTIVEC_FLAG" != "x"; then
   echo "CRYPTOPP_ALTIVEC_FLAG: $CRYPTOPP_ALTIVEC_FLAG"
fi
if test x"$CRYPTOPP_POWER7_FLAG" != "x"; then
   echo "CRYPTOPP_POWER7_FLAG: $CRYPTOPP_POWER7_FLAG"
fi
if test x"$CRYPTOPP_POWER8_FLAG" != "x"; then
   echo "CRYPTOPP_POWER8_FLAG: $CRYPTOPP_POWER8_FLAG"
fi
#if test x"$CRYPTOPP_POWER9_FLAG" != "x"; then
#   echo "CRYPTOPP_POWER9_FLAG: $CRYPTOPP_POWER9_FLAG"
#fi
if test x"$CRYPTOPP_ARIA_FLAG" != "x"; then
   echo "CRYPTOPP_ARIA_FLAG: $CRYPTOPP_ARIA_FLAG"
fi
if test x"$CRYPTOPP_BLAKE2S_FLAG" != "x"; then
   echo "CRYPTOPP_BLAKE2S_FLAG: $CRYPTOPP_BLAKE2S_FLAG"
fi
if test x"$CRYPTOPP_BLAKE2B_FLAG" != "x"; then
   echo "CRYPTOPP_BLAKE2B_FLAG: $CRYPTOPP_BLAKE2B_FLAG"
fi
if test x"$CRYPTOPP_CHACHA_FLAG" != "x"; then
   echo "CRYPTOPP_CHACHA_FLAG: $CRYPTOPP_CHACHA_FLAG"
fi
if test x"$CRYPTOPP_CHACHA_AVX2_FLAG" != "x"; then
   echo "CRYPTOPP_CHACHA_AVX2_FLAG: $CRYPTOPP_CHACHA_AVX2_FLAG"
fi
if test x"$CRYPTOPP_CHAM_FLAG" != "x"; then
   echo "CRYPTOPP_CHAM_FLAG: $CRYPTOPP_CHAM_FLAG"
fi
if test x"$CRYPTOPP_CRC_FLAG" != "x"; then
   echo "CRYPTOPP_CRC_FLAG: $CRYPTOPP_CRC_FLAG"
fi
if test x"$CRYPTOPP_DONNA_FLAG" != "x"; then
   echo "CRYPTOPP_DONNA_FLAG: $CRYPTOPP_DONNA_FLAG"
fi
if test x"$CRYPTOPP_LEA_FLAG" != "x"; then
   echo "CRYPTOPP_LEA_FLAG: $CRYPTOPP_LEA_FLAG"
fi
if test x"$CRYPTOPP_KECCAK_FLAG" != "x"; then
   echo "CRYPTOPP_KECCAK_FLAG: $CRYPTOPP_KECCAK_FLAG"
fi
if test x"$CRYPTOPP_GCM_FLAG" != "x"; then
   echo "CRYPTOPP_GCM_FLAG: $CRYPTOPP_GCM_FLAG"
fi
if test x"$CRYPTOPP_GF2N_FLAG" != "x"; then
   echo "CRYPTOPP_GF2N_FLAG: $CRYPTOPP_GF2N_FLAG"
fi
if test x"$CRYPTOPP_AES_FLAG" != "x"; then
   echo "CRYPTOPP_AES_FLAG: $CRYPTOPP_AES_FLAG"
fi
if test x"$CRYPTOPP_SHA_FLAG" != "x"; then
   echo "CRYPTOPP_SHA_FLAG: $CRYPTOPP_SHA_FLAG"
fi
if test x"$CRYPTOPP_SIMON128_FLAG" != "x"; then
   echo "CRYPTOPP_SIMON128_FLAG: $CRYPTOPP_SIMON128_FLAG"
fi
if test x"$CRYPTOPP_SPECK128_FLAG" != "x"; then
   echo "CRYPTOPP_SPECK128_FLAG: $CRYPTOPP_SPECK128_FLAG"
fi
if test x"$CRYPTOPP_SM4_FLAG" != "x"; then
   echo "CRYPTOPP_SM4_FLAG: $CRYPTOPP_SM4_FLAG"
fi
if test x"$CRYPTOPGAMS_ARMV4_FLAG" != "x"; then
   echo "CRYPTOPGAMS_ARMV4_FLAG: $CRYPTOPGAMS_ARMV4_FLAG"
fi

echo ""
echo "Automake flags (can be overridden by user flags):"
echo "AM_CXXFLAGS: $AM_CXXFLAGS"
echo "AM_LDFLAGS: $AM_LDFLAGS"

echo ""
echo "User flags (overrides Automake flags on conflict):"
echo "CXXFLAGS: $CXXFLAGS"
echo "LDFLAGS: $LDFLAGS"

OPT_NDEBUG=`echo $CPPFLAGS $CXXFLAGS | $EGREP -c '\-DNDEBUG'`
if test "$OPT_NDEBUG" = "0"; then
   echo ""
   echo "CPPFLAGS does not include -DNDEBUG. You should consider building"
   echo "with NDEBUG defined so an assert does not inadvertently crash your"
   echo "program and egress sensitive data to the filesystem or an error"
   echo "reporting service like AppPort, Windows Error Reporting, Crash"
   echo "Reporter, Test Flight, etc."
fi

OPT_SYM=`echo $CPPFLAGS $CXXFLAGS | $EGREP -c '\-g'`
if test "$OPT_SYM" = "0"; then
   echo ""
   echo "CPPFLAGS does not include -g. You should consider building with -g"
   echo "to add debug symbols in case you have to analyze a crash."
fi

OPT_O3=`echo $CXXFLAGS | $EGREP -c '\-O3|\-O4|\-O5|\-xO3|\-xO4|\-xO5|\-Os|\-Ofast'`
if test "$OPT_O3" = "0"; then
   echo ""
   echo "CXXFLAGS does not include -O3. You should consider building at -O3"
   echo "to engage compiler vectorizations and enhance performance."
fi

echo "***********************************************************************"
echo ""
