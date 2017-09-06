dnl as-version.m4 0.2.0

dnl autostars m4 macro for versioning

dnl Thomas Vander Stichele <thomas at apestaart dot org>

dnl $Id: as-version.m4,v 1.4 2004/06/01 09:40:05 thomasvs Exp $

dnl AS_VERSION

dnl example
dnl AS_VERSION

dnl this macro
dnl - AC_SUBST's PACKAGE_VERSION_MAJOR, _MINOR, _MICRO
dnl - AC_SUBST's PACKAGE_VERSION_RELEASE,
dnl    which can be used for rpm release fields
dnl - doesn't call AM_INIT_AUTOMAKE anymore because it prevents
dnl   maintainer mode from running correctly
dnl
dnl don't forget to put #undef PACKAGE_VERSION_RELEASE in acconfig.h
dnl if you use acconfig.h

AC_DEFUN([AS_VERSION],
[
  PACKAGE_VERSION_MAJOR=$(echo AC_PACKAGE_VERSION | cut -d'.' -f1)
  PACKAGE_VERSION_MINOR=$(echo AC_PACKAGE_VERSION | cut -d'.' -f2)
  PACKAGE_VERSION_MICRO=$(echo AC_PACKAGE_VERSION | cut -d'.' -f3)

  AC_SUBST(PACKAGE_VERSION_MAJOR)
  AC_SUBST(PACKAGE_VERSION_MINOR)
  AC_SUBST(PACKAGE_VERSION_MICRO)
])

dnl AS_NANO(ACTION-IF-NO-NANO, [ACTION-IF-NANO])

dnl requires AC_INIT to be called before
dnl For projects using a fourth or nano number in your versioning to indicate
dnl development or prerelease snapshots, this macro allows the build to be
dnl set up differently accordingly.

dnl this macro:
dnl - parses AC_PACKAGE_VERSION, set by AC_INIT, and extracts the nano number
dnl - sets the variable PACKAGE_VERSION_NANO
dnl - sets the variable PACKAGE_VERSION_RELEASE, which can be used
dnl   for rpm release fields
dnl - executes ACTION-IF-NO-NANO or ACTION-IF-NANO
    
dnl example:
dnl AS_NANO(RELEASE="yes", RELEASE="no")

AC_DEFUN([AS_NANO],
[
  AC_MSG_CHECKING(nano version)

  NANO=$(echo AC_PACKAGE_VERSION | cut -d'.' -f4)

  if test x"$NANO" = x || test "x$NANO" = "x0" ; then
    AC_MSG_RESULT([0 (release)])
    NANO=0
    PACKAGE_VERSION_RELEASE=1
    ifelse([$1], , :, [$1])
  else
    AC_MSG_RESULT($NANO)
    PACKAGE_VERSION_RELEASE=0.`date +%Y%m%d.%H%M%S`
    ifelse([$2], , :, [$2])
  fi
  PACKAGE_VERSION_NANO=$NANO
  AC_SUBST(PACKAGE_VERSION_NANO)
  AC_SUBST(PACKAGE_VERSION_RELEASE)
])
