AUTOMAKE_OPTIONS = foreign

pkgdir = $(datadir)/amule
ACLOCAL_AMFLAGS = -I m4

pkg_LTLIBRARIES = libantiLeech.la
libantiLeech_la_CPPFLAGS = ${ANTILEECH_CPPFLAGS}
libantiLeech_la_LDFLAGS = ${ANTILEECH_LDFLAGS} -module -avoid-version --no-la-files
libantiLeech_la_SOURCES = \
	antiLeech.h \
	antiLeech_wx.h \
	CString_wx.h \
	antiLeech.cpp \
	antiLeech_wx.cpp \
	Interface.cpp

