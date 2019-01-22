VERSION=1

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

OS=$(shell uname)
ifeq ($(OS),Darwin)
  LINK=$(LD)
  SHLIB_EXT=dylib
  SHLIB_FLAGS=-dylib
  SHLIB_FILE=libiptbwctl.$(SHLIB_EXT).$(VERSION)
else
  LINK=$(CC)
  SHLIB_EXT=so
  SHLIB_FILE=libiptbwctl.$(SHLIB_EXT).$(VERSION)
  SHLIB_FLAGS=-shared -Wl,-soname,$(SHLIB_FILE)
endif

CFLAGS:=$(CFLAGS) -Os
WARNING_FLAGS=-Wall -Wstrict-prototypes

all: libiptbwctl

libiptbwctl: libiptbwctl.$(SHLIB_EXT) libiptbwctl.a

libiptbwctl.a: ipt_bwctl_static.o ipt_bwctl_safe_malloc_static.o
	if [ -e $@ ] ; then rm $@ ; fi
	$(AR) rc $@ $^
	$(RANLIB) $@

libiptbwctl.$(SHLIB_EXT) : ipt_bwctl_dyn.o ipt_bwctl_safe_malloc_dyn.o
	if [ -e libiptbwctl.$(SHLIB_EXT) ] ; then rm libiptbwctl.$(SHLIB_EXT)* ; fi
	$(LINK) $(LDFLAGS) $(SHLIB_FLAGS) -o $(SHLIB_FILE) $^ -lc
	ln -s $(SHLIB_FILE) libiptbwctl.$(SHLIB_EXT)

%_dyn.o: %.c
	$(CC) $(CFLAGS) -fPIC $(WARNING_FLAGS) -o $@ -c $^

%_static.o: %.c
	$(CC) $(CFLAGS) $(WARNING_FLAGS) -o $@ -c $^

clean:
	cd utils
	rm -rf bw_get bw_set *.a *.o *~ .*sw*
	cd ..
	if [ -n "$(SHLIB_EXT)" ] ; then rm -rf *.$(SHLIB_EXT)* ; fi
	rm -rf *.a *.o *~ .*sw*
