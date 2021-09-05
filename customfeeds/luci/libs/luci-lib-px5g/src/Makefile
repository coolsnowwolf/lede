PX5G_CFLAGS = -I. -include polarssl/rsa.h -include polarssl/x509.h -std=gnu99
PX5G_LDFLAGS = -llua -lm
PX5G_OBJ = px5g.o library/bignum.o library/havege.o library/rsa.o library/sha1.o library/timing.o library/x509write.o
PX5G_LIB = px5g.so

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LUA_CFLAGS) $(PX5G_CFLAGS) $(FPIC) -c -o $@ $<

compile: $(PX5G_OBJ)
	$(CC) $(LDFLAGS) -shared -o $(PX5G_LIB) $(PX5G_OBJ) $(PX5G_LDFLAGS)

install: compile
	mkdir -p $(DESTDIR)/usr/lib/lua
	cp $(PX5G_LIB) $(DESTDIR)/usr/lib/lua/$(PX5G_LIB)

clean:
	rm -f *.o *.so
