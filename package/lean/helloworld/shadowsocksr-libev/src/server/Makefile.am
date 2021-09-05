VERSION_INFO = 2:0:0

AM_CFLAGS = -g -O2 -Wall -Werror -Wno-deprecated-declarations -fno-strict-aliasing -std=gnu99 -D_GNU_SOURCE
AM_CFLAGS += $(PTHREAD_CFLAGS)
if !USE_SYSTEM_SHARED_LIB
AM_CFLAGS += -I$(top_srcdir)/libev
AM_CFLAGS += -I$(top_srcdir)/libudns
AM_CFLAGS += -I$(top_srcdir)/libsodium/src/libsodium/include
endif
AM_CFLAGS += -I$(top_srcdir)/libipset/include
AM_CFLAGS += -I$(top_srcdir)/libcork/include
AM_CFLAGS += $(LIBPCRE_CFLAGS)

SS_COMMON_LIBS = $(top_builddir)/libipset/libipset.la \
                 $(top_builddir)/libcork/libcork.la \
                 $(INET_NTOP_LIB) $(LIBPCRE_LIBS)
if USE_SYSTEM_SHARED_LIB
SS_COMMON_LIBS += -lev -lsodium -lm
else
SS_COMMON_LIBS += $(top_builddir)/libev/libev.la \
                  $(top_builddir)/libsodium/src/libsodium/libsodium.la
endif

bin_PROGRAMS = ss-server ss-check

sni_src = http.c \
          tls.c \
          rule.c

ss_check_SOURCES = check.c 

ss_server_SOURCES = utils.c \
                    netutils.c \
                    jconf.c \
                    json.c \
                    encrypt.c \
                    udprelay.c \
                    cache.c \
                    acl.c \
                    resolv.c \
                    server.c \
                    $(sni_src)


ss_check_LDADD = $(SS_COMMON_LIBS)
ss_server_LDADD = $(SS_COMMON_LIBS)

if USE_SYSTEM_SHARED_LIB
ss_server_LDADD += -ludns
else
ss_server_LDADD += $(top_builddir)/libudns/libudns.la
endif

ss_check_CFLAGS = $(AM_CFLAGS) 
ss_server_CFLAGS = $(AM_CFLAGS) -DMODULE_REMOTE
