CC = gcc
CFLAGS = -Wall
OBJS = main.o

all: maccalc

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

maccalc: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f maccalc *.o
