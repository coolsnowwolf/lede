###############################################################################
# Gather all flags.
#
CFLAGS+=-DPOSIXLY_CORRECT -Wall -g

TARGET=cpulimit-ng

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o, $(SOURCES))

$(TARGET) : $(OBJS)
	$(CC) `for X in $(OBJS) ; do echo "$$X"; done` -o $(TARGET) $(LIBS) $(LDFLAGS)

all: $(TARGET)

clean:
	rm -f *.o $(TARGET)
