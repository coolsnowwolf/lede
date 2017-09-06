CC = gcc
CFLAGS = -Wall
OBJS = main.o cyg_crc32.o

all: rbcfg

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

rbcfg: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f rbcfg *.o
