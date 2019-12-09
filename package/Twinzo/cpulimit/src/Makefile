BINDIR     ?= $(PREFIX)/bin
BIN_TARGET  = cpulimit
CC         ?=  gcc
CFLAGS     ?= -O2 -Wall -g -D_GNU_SOURCE
HELP2MAN   := $(shell which help2man)
LIBS        = list.o process_iterator.o process_group.o
MAN_NAME   := "CPU Utilization Limiter"
MAN_SEC    := 1
MAN_SRC    := $(shell git remote get-url origin)
MAN_TARGET  = $(BIN_TARGET).$(MAN_SEC)
PREFIX     ?= /usr/local
STRIP      := $(shell which strip)
SUDO       := $(shell which sudo)
TARGETS     = $(BIN_TARGET) $(MAN_TARGET)
UNAME      := $(shell uname)

ifeq ($(UNAME), FreeBSD)
LIBS       += -lkvm
endif

all::	$(TARGETS) $(LIBS)

static:
	$(MAKE) all CFLAGS="$(CFLAGS) --static"

strip: $(BIN_TARGET)
	$(STRIP) $<

install: $(TARGETS)
	$(SUDO) install $^ $(BINDIR)

$(BIN_TARGET):	cpulimit.c $(LIBS)
	$(CC) -o cpulimit cpulimit.c $(LIBS) $(CFLAGS)

$(MAN_TARGET): $(BIN_TARGET) $(HELP2MAN)
	$(HELP2MAN) -n $(MAN_NAME) -s $(MAN_SEC) -s $(MAN_SRC) -N ./$< > $@

process_iterator.o: process_iterator.c process_iterator.h
	$(CC) -c process_iterator.c $(CFLAGS)

list.o: list.c list.h
	$(CC) -c list.c $(CFLAGS)

process_group.o: process_group.c process_group.h process_iterator.o list.o
	$(CC) -c process_group.c $(CFLAGS)

clean:
	rm -f *~ *.o $(TARGETS)

