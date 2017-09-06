# stripped intltool.m4 to make automake happy
# serial 1
AC_DEFUN([IT_PROG_INTLTOOL],
[
	AC_SUBST(ALL_LINGUAS)
	
	DATADIRNAME=share
	AC_SUBST(DATADIRNAME)
])

# deprecated macros
AU_ALIAS([AC_PROG_INTLTOOL], [IT_PROG_INTLTOOL])

# A hint is needed for aclocal from Automake <= 1.9.4:
# AC_DEFUN([AC_PROG_INTLTOOL], ...)

