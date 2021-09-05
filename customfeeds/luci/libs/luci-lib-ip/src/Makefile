IP_CFLAGS = -std=gnu99 -I$(STAGING_DIR)/usr/include/libnl-tiny/
IP_LDFLAGS = -llua -lm -lnl-tiny
IP_OBJ = ip.o
IP_LIB = ip.so

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LUA_CFLAGS) $(IP_CFLAGS) $(FPIC) -c -o $@ $<

compile: $(IP_OBJ)
	$(CC) $(LDFLAGS) -shared -o $(IP_LIB) $(IP_OBJ) $(IP_LDFLAGS)

install: compile
	mkdir -p $(DESTDIR)/usr/lib/lua/luci
	cp $(IP_LIB) $(DESTDIR)/usr/lib/lua/luci/$(IP_LIB)

clean:
	rm -f *.o *.so
