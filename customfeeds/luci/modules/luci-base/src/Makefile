%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FPIC) -c -o $@ $<

clean:
	rm -f po2lmo parser.so version.lua *.o

po2lmo: po2lmo.o template_lmo.o
	$(CC) $(LDFLAGS) -o $@ $^

parser.so: template_parser.o template_utils.o template_lmo.o template_lualib.o
	$(CC) $(LDFLAGS) -shared -o $@ $^

version.lua:
	./mkversion.sh $@ $(LUCI_VERSION) "$(LUCI_GITBRANCH)"

compile: parser.so version.lua

install: compile
	mkdir -p $(DESTDIR)/usr/lib/lua/luci/template
	cp parser.so $(DESTDIR)/usr/lib/lua/luci/template/parser.so
	cp version.lua $(DESTDIR)/usr/lib/lua/luci/version.lua
