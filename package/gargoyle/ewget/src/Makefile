

VERSION=1

#for ssl, uncomment the (one and only one) ssl library to link with
#USE_MATRIXSSL:=1
#USE_CYASSL:=1
#USE_MBEDTLS:=1
USE_OPENSSL:=1

#uncomment to link dynamically to ericstools
#USE_ERICSTOOLS:=1


ifeq ($(CC),)
  CC=gcc
endif

ifeq ($(LD),)
  LD=ld
endif

ifeq ($(AR),)
  AR=ar
endif

ifeq ($(RANLIB),)
  RANLIB=ranlib
endif


CFLAGS:=$(CFLAGS) -Os

WARNING_FLAGS=-Wall -Wstrict-prototypes -pedantic
MINIMAL_WARNING_FLAGS=-Wall -Wstrict-prototypes

OS=$(shell uname)
ifeq ($(OS),Darwin)
  LINK=$(LD)
  SHLIB_EXT=dylib
  SHLIB_FLAGS=-dylib
  SHLIB_FILE=libewget.$(SHLIB_EXT).$(VERSION)
  CFLAGS:=$(CFLAGS) -arch i386
else
  LINK=$(CC)
  SHLIB_EXT=so
  SHLIB_FILE=libewget.$(SHLIB_EXT).$(VERSION)
  SHLIB_FLAGS=-shared -Wl,-soname,$(SHLIB_FILE)
endif



LIBS:=
SSL_DEFS:=
SSL_OBJ_DYN:=
SSL_OBJ_STATIC:=


ifeq ($(USE_MBEDTLS), 1)
	SSL_DEFS:= -DUSE_MBEDTLS -DHAVE_SSL
	LIBS:=$(LIBS) -lmbedtls
	USE_MATRIXSSL=
	USE_CYASSL=
	USE_OPENSSL=
endif
ifeq ($(USE_CYASSL), 1)
	SSL_DEFS:= -DUSE_CYASSL -DHAVE_SSL
	ifeq ($(USE_CYASSL_INCLUDE_DIR), 1)
		SSL_DEFS:=$(SSL_DEFS) -DUSE_CYASSL_INCLUDE_DIR
	endif
	LIBS:=$(LIBS) -lcyassl -lm
	USE_MATRIXSSL=
	USE_OPENSSL=
	USE_MBEDTLS=
endif
ifeq ($(USE_MATRIXSSL), 1)
	SSL_DEFS:= -DUSE_MATRIXSSL  -DHAVE_SSL 
	LIBS:=$(LIBS) -lmatrixssl
	SSL_OBJ_DYN=matrixssl_helper_dyn.o
	SSL_OBJ_STATIC=matrixssl_helper_static.o
	USE_CYASSL=
	USE_OPENSSL=
	USE_MBEDTLS=
	
endif
ifeq ($(USE_OPENSSL), 1)
	SSL_DEFS:= -DUSE_OPENSSL -DHAVE_SSL
	LIBS:=$(LIBS) -lcrypto -lssl
	USE_MATRIXSSL=
	USE_CYASSL=
	USE_MBEDTLS=
endif

ifeq ($(USE_ERICSTOOLS), 1)
	LIBS:=$(LIBS) -lericstools
	SSL_DEFS:=$(SSL_DEFS) -DUSE_ERICSTOOLS
endif


all: libewget.a libewget.$(SHLIB_EXT)



libewget.a: libewget_static.o $(SSL_OBJ_STATIC)
	if [ -e $@ ] ; then rm $@ ; fi
	$(AR) rc $@ $^
	$(RANLIB) $@
	

libewget.$(SHLIB_EXT) : libewget_dyn.o $(SSL_OBJ_DYN)
	if [ -e libewget.$(SHLIB_EXT) ] ; then rm libewget.$(SHLIB_EXT)* ; fi
	$(LINK) $(LDFLAGS) $(SHLIB_FLAGS) -o $(SHLIB_FILE) $^ -lc $(LIBS)
	ln -s $(SHLIB_FILE) libewget.$(SHLIB_EXT)


%_dyn.o: %.c
	$(CC) $(CFLAGS) -fPIC $(WARNING_FLAGS) $(SSL_DEFS) -o $@ -c $^

%_static.o: %.c
	$(CC) $(CFLAGS) $(WARNING_FLAGS) $(SSL_DEFS) -o $@ -c $^



clean:
	rm -rf *.a *.o .*sw* *~ 
	if [ "$(SHLIB_EXT)" != "" ] ; then rm -rf *.$(SHLIB_EXT)* ; fi
install:
	cp *.h /usr/include
	cp *.$(SHLIB_EXT)* /usr/lib
