NEIGHTBL_CFLAGS = -std=gnu99
NEIGHTBL_LDFLAGS = -llua -lm
NEIGHTBL_OBJ = neightbl.o
NEIGHTBL_LIB = neightbl.so

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LUA_CFLAGS) $(NEIGHTBL_CFLAGS) $(FPIC) -c -o $@ $<

compile: $(NEIGHTBL_OBJ)
	$(CC) $(LDFLAGS) -shared -o $(NEIGHTBL_LIB) $(NEIGHTBL_OBJ) $(NEIGHTBL_LDFLAGS)

install: compile
	mkdir -p $(DESTDIR)/usr/lib/lua
	cp $(NEIGHTBL_LIB) $(DESTDIR)/usr/lib/lua/$(NEIGHTBL_LIB)

clean:
	rm -f *.o *.so
