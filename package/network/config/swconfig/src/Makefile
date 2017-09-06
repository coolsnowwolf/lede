ifndef CFLAGS
CFLAGS = -O2 -g -I ../src
endif
LIBS=-lnl -lnl-genl

all: swconfig

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $^

libsw.a: swlib.o
	$(AR) rcu $@ swlib.o
	$(RANLIB) $@

swconfig: libsw.a cli.o uci.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) -L./ -lsw
