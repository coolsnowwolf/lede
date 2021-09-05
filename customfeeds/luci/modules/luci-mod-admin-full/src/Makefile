%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FPIC) -c -o $@ $<

clean:
	rm -f luci-bwc *.o

luci-bwc: luci-bwc.o
	$(CC) $(LDFLAGS) -o $@ $^ -ldl

compile: luci-bwc

install: compile
	mkdir -p $(DESTDIR)/usr/bin
	cp luci-bwc $(DESTDIR)/usr/bin/luci-bwc
