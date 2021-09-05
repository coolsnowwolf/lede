CC ?= gcc
CFLAGS ?= -O2 -Wall -pedantic
CFLAGS += -std=gnu99

.PHONY: clean

sockread:
	$(CC) $(CFLAGS) main.c -o sockread

clean:
	rm -f sockread
