
#uncomment if using cyassl
#USE_CYASSL=1

OS=$(shell uname)
ifeq ($(OS),Darwin)
  CFLAGS:=$(CFLAGS) -arch i386
endif

LIBS:= -lewget
ifeq ($(USE_CYASSL), 1)
  LIBS:=$(LIBS) -lcyassl 
endif

all: ewget

ewget: ewget.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^  -o $@ $(LIBS)

ewget.o: ewget.c
	$(CC) $(CFLAGS) -c $^ -o $@ 

clean:
	rm -rf ewget *.o *~ .*sw* 
