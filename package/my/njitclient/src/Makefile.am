ACLOCAL_AMFLAGS = -I m4
sbin_PROGRAMS = \
		client \
		$(NULL)
client_SOURCES = \
		main.c \
		auth.c \
		ip.c   \
		fillmd5-libcrypto.c \
		njit8021xclient.c \
		njit8021xclient.h \
		debug.h \
		md5-buildin/md5_one.c \
		md5-buildin/md5_dgst.c \
		md5-buildin/mem_clr.c \
		aes.h \
		aes.c \
		h3c_AES_MD5.h \
		h3c_AES_MD5.c \
		h3c_dict.h \
		$(NULL)
CCOPT = $(V_CCOPT)
DEFS  = $(V_DEFS)
INCLS = $(V_INCLS)
client_CFLAGS = $(CCOPT) $(DEFS) $(INCLS)
client_CFLAGS += $(libcrypto_CFLAGS)
client_LDADD  = $(LBL_LIBS)

