all: bw_convert

bw_convert: bw_convert.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -lericstools

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -rf *.o *~ .*sw* bw_convert
