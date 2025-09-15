CC = gcc
CFLAGS =
WFLAGS = -Wall -Werror
padjffs2-objs = padjffs2.o

all: padjffs2

%.o: %.c
	$(CC) $(CFLAGS) $(WFLAGS) -c -o $@ $<

padjffs2: $(padjffs2-objs)
	$(CC) $(LDFLAGS) -o $@ $(padjffs2-objs)

clean:
	rm -f padjffs2 *.o
