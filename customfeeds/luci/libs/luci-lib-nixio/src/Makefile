OS            = Linux
AXTLS_VERSION = 1.2.1
AXTLS_DIR     = axTLS
AXTLS_FILE    = $(AXTLS_DIR)-$(AXTLS_VERSION).tar.gz
#NIXIO_TLS    ?= openssl
NIXIO_SHADOW ?= $(shell echo 'int main(void){ return !getspnam("root"); }' | $(CC) $(CFLAGS) -include shadow.h -xc -o/dev/null - 2>/dev/null && echo yes)
NIXIO_SO      = nixio.so
NIXIO_LDFLAGS = -llua -lm -ldl
CFLAGS       += -std=gnu99

ifeq (,$(findstring Darwin,$(OS)))
	NIXIO_LDFLAGS += -lcrypt -shared
else
        NIXIO_LDFLAGS += -bundle -undefined dynamic_lookup
	EXTRA_CFLAGS += -D__DARWIN__
endif

NIXIO_OBJ = nixio.o socket.o sockopt.o bind.o address.o \
	    protoent.o poll.o io.o file.o splice.o process.o \
	    syslog.o bit.o binary.o fs.o user.o \
	    $(if $(NIXIO_TLS),tls-crypto.o tls-context.o tls-socket.o,)

ifeq ($(NIXIO_TLS),axtls)
	TLS_CFLAGS = -IaxTLS/ssl -IaxTLS/crypto -IaxTLS/config -include axtls-compat.h
	TLS_DEPENDS = axtls-compat.o
	NIXIO_OBJ += axtls-compat.o libaxtls.a
endif

ifeq ($(NIXIO_TLS),openssl)
	NIXIO_LDFLAGS += -lssl -lcrypto
endif

ifeq ($(NIXIO_TLS),cyassl)
	NIXIO_LDFLAGS += -lcyassl
	TLS_DEPENDS = cyassl-compat.o
	TLS_CFLAGS = -include cyassl-compat.h
	NIXIO_OBJ += cyassl-compat.o
endif

ifeq ($(NIXIO_TLS),)
	NIXIO_CFLAGS += -DNO_TLS
endif

ifneq ($(NIXIO_SHADOW),yes)
	NIXIO_CFLAGS += -DNO_SHADOW
endif


ifeq ($(OS),SunOS)
	NIXIO_LDFLAGS += -lsocket -lnsl -lsendfile
endif

ifneq (,$(findstring MINGW,$(OS))$(findstring mingw,$(OS))$(findstring Windows,$(OS)))
	NIXIO_CROSS_CC:=$(shell which i586-mingw32msvc-cc)
ifneq (,$(NIXIO_CROSS_CC))
	CC:=$(NIXIO_CROSS_CC)
endif
	NIXIO_OBJ += mingw-compat.o
	NIXIO_LDFLAGS_POST:=-llua -lssl -lcrypto -lws2_32 -lgdi32
	FPIC:=
	EXTRA_CFLAGS += -D_WIN32_WINNT=0x0501
	LUA_CFLAGS:=
	NIXIO_SO:=nixio.dll
	NIXIO_LDFLAGS:=
endif


%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(NIXIO_CFLAGS) $(LUA_CFLAGS) $(FPIC) -c -o $@ $< 

ifneq ($(NIXIO_TLS),)
tls-crypto.o: $(TLS_DEPENDS) tls-crypto.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(NIXIO_CFLAGS) $(LUA_CFLAGS) $(FPIC) $(TLS_CFLAGS) -c -o $@ tls-crypto.c

tls-context.o: $(TLS_DEPENDS) tls-context.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(NIXIO_CFLAGS) $(LUA_CFLAGS) $(FPIC) $(TLS_CFLAGS) -c -o $@ tls-context.c
	
tls-socket.o: $(TLS_DEPENDS) tls-socket.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(NIXIO_CFLAGS) $(LUA_CFLAGS) $(FPIC) $(TLS_CFLAGS) -c -o $@ tls-socket.c
	
axtls-compat.o: libaxtls.a axtls-compat.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(NIXIO_CFLAGS) $(LUA_CFLAGS) $(FPIC) $(TLS_CFLAGS) -c -o $@ axtls-compat.c
	mkdir -p dist
	cp -pR axtls-root/* dist/
endif	

compile: $(NIXIO_OBJ)
	$(CC) $(LDFLAGS) $(SHLIB_FLAGS) -o $(NIXIO_SO) $(NIXIO_OBJ) $(NIXIO_LDFLAGS) $(NIXIO_LDFLAGS_POST)
	mkdir -p dist/usr/lib/lua
	cp $(NIXIO_SO) dist/usr/lib/lua/$(NIXIO_SO)

$(AXTLS_DIR)/.prepared:
	#rm -rf $(AXTLS_DIR)
	#tar xvfz $(AXTLS_FILE)
	cp axtls-config/.config axtls-config/config.h $(AXTLS_DIR)/config
	touch $@

libaxtls.a: $(AXTLS_DIR)/.prepared
	$(MAKE) -C $(AXTLS_DIR) CC="$(CC)" CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS) $(FPIC) -Wall -pedantic -I../config -I../ssl -I../crypto" LDFLAGS="$(LDFLAGS)" OS="$(OS)" clean all
	cp -p $(AXTLS_DIR)/_stage/libaxtls.a src
	# *************************************************************************
	#
	#
	#
	# *** WARNING ***
	# The use of the axTLS cryptographical provider is discouraged!
	# Please switch to either CyaSSL or OpenSSL.
	# Support for axTLS might be removed in the near future.
	#
	#
	#
	#**************************************************************************

clean:
	rm -f *.o *.so *.a *.dll
	rm -f $(AXTLS_DIR)/.prepared

install: compile
	mkdir -p $(DESTDIR)
	cp -pR dist/* $(DESTDIR)/
