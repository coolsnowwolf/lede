## Process this file with automake to produce Makefile.in
# We must define a dummy directory for the backenddir so make discheck works
AM_DISTCHECK_CONFIGURE_FLAGS = --with-cupsbackenddir=/tmp

cupsbackend_PROGRAMS = bjnp
bjnp_SOURCES = \
	bjnp.c \
	bjnp-runloop.c \
	bjnp-io.c \
	bjnp-debug.c \
	bjnp-utils.c \
	bjnp-discover.c \
	bjnp-commands.c \
	bjnp-levels.c \
	cups-bjnp.spec \
	bjnp.h \
	bjnp-commands.h \
	bjnp-protocol.h \
	bjnp-io.h \
	TODO \
	README \
	README.levels \
	COPYING
bjnp_CFLAGS = -Wall $(CUPS_CFLAGS) $(EXTRA_CFLAGS) 
bjnp_LDFLAGS =  $(CUPS_LDFLAGS)
bjnp_LDADD =  $(CUPS_LIBS)

