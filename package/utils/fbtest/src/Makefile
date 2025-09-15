CC = gcc
CFLAGS = -Wall
OBJS = fbtest.o

all: fbtest

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

fbtest: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f fbtest *.o
