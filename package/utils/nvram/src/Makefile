all: nvram

nvram:
	$(CC) $(CFLAGS) -o $@ cli.c crc.c nvram.c $(LDFLAGS)

clean:
	rm -f nvram
