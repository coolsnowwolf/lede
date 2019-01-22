ACLOCAL_AMFLAGS = -I m4
SUBDIRS = \
          tests \
          po \
          $(NULL)
EXTRA_DIST = HACKING

bin_PROGRAMS = hello
hello_SOURCES = hello.c hello.h config.c config.h
hello_CPPFLAGS = @GTK_CFLAGS@
hello_LDADD = @GTK_LIBS@

