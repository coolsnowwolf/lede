CC       = gcc
CPPFLAGS = -I. -Itinysrp
CFLAGS   = -Os -Wall
LDFLAGS	 =
LIBS_EADCLIENT = tinysrp/libtinysrp.a
LIBS_EAD = tinysrp/libtinysrp.a -lpcap
CONFIGURE_ARGS =

all: ead ead-client

obj = ead-crypt.o libbridge_init.o

tinysrp/Makefile:
	cd tinysrp; ./configure $(CONFIGURE_ARGS)

tinysrp/libtinysrp.a: tinysrp/Makefile
	-$(MAKE) -C tinysrp CFLAGS="$(CFLAGS)"

%.o: %.c $(wildcard *.h) tinysrp/libtinysrp.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

ead.o: filter.c
ead-crypt.o: aes.c sha1.c

ead: ead.o $(obj) tinysrp/libtinysrp.a
	$(CC) -o $@ $< $(obj) $(LDFLAGS) $(LIBS_EAD)

ead-client: ead-client.o $(obj)
	$(CC) -o $@ $< $(obj) $(LDFLAGS) $(LIBS_EADCLIENT)

clean:
	rm -f *.o ead ead-client
	if [ -f tinysrp/Makefile ]; then $(MAKE) -C tinysrp distclean; fi
