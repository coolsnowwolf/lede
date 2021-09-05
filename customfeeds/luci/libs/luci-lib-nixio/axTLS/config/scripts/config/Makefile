# Makefile for axTLS
#
# Copyright (C) 2002 Erik Andersen <andersen@codepoet.org>

top_srcdir=../..
top_builddir=../..
srcdir=$(top_srcdir)/scripts/config
include $(top_srcdir)/Rules.mak

all: ncurses conf mconf

ifeq ($(shell uname),SunOS)
LIBS = -lcurses
else
LIBS = -lncurses
endif
ifeq (/usr/include/ncurses/ncurses.h, $(wildcard /usr/include/ncurses/ncurses.h))
	HOSTNCURSES += -I/usr/include/ncurses -DCURSES_LOC="<ncurses.h>"
else
ifeq (/usr/include/ncurses/curses.h, $(wildcard /usr/include/ncurses/curses.h))
	HOSTNCURSES += -I/usr/include/ncurses -DCURSES_LOC="<ncurses/curses.h>"
else
ifeq (/usr/local/include/ncurses/ncurses.h, $(wildcard /usr/local/include/ncurses/ncurses.h))
	HOSTCFLAGS += -I/usr/local/include/ncurses -DCURSES_LOC="<ncurses.h>"
else
ifeq (/usr/local/include/ncurses/curses.h, $(wildcard /usr/local/include/ncurses/curses.h))
	HOSTCFLAGS += -I/usr/local/include/ncurses -DCURSES_LOC="<ncurses/curses.h>"
else
ifeq (/usr/include/ncurses.h, $(wildcard /usr/include/ncurses.h))
	HOSTNCURSES += -DCURSES_LOC="<ncurses.h>"
else
	HOSTNCURSES += -DCURSES_LOC="<curses.h>"
endif
endif
endif
endif
endif

CONF_SRC     = conf.c
MCONF_SRC    = mconf.c
LXD_SRC      = lxdialog/checklist.c lxdialog/menubox.c lxdialog/textbox.c \
               lxdialog/yesno.c lxdialog/inputbox.c lxdialog/util.c \
               lxdialog/msgbox.c

SHARED_SRC   = zconf.tab.c
SHARED_DEPS := $(srcdir)/lkc.h $(srcdir)/lkc_proto.h \
               lkc_defs.h $(srcdir)/expr.h zconf.tab.h
CONF_OBJS    = $(patsubst %.c,%.o, $(CONF_SRC))
MCONF_OBJS   = $(patsubst %.c,%.o, $(MCONF_SRC) $(LXD_SRC))
SHARED_OBJS  = $(patsubst %.c,%.o, $(SHARED_SRC))

conf: $(CONF_OBJS) $(SHARED_OBJS)
	$(HOSTCC) $(NATIVE_LDFLAGS) $^ -o $@

mconf: $(MCONF_OBJS) $(SHARED_OBJS)
	$(HOSTCC) $(NATIVE_LDFLAGS) $^ -o $@ $(LIBS)

$(CONF_OBJS): %.o : $(srcdir)/%.c $(SHARED_DEPS)
	$(HOSTCC) $(HOSTCFLAGS) -I. -c $< -o $@

$(MCONF_OBJS): %.o : $(srcdir)/%.c $(SHARED_DEPS)
	@[ -d $(@D) ] || mkdir -v $(@D)
	$(HOSTCC) $(HOSTCFLAGS) $(HOSTNCURSES) -I. -c $< -o $@

lkc_defs.h: $(srcdir)/lkc_proto.h
	@sed < $< > $@ 's/P(\([^,]*\),.*/#define \1 (\*\1_p)/'

###
# The following requires flex/bison
# By default we use the _shipped versions, uncomment the
# following line if you are modifying the flex/bison src.
#LKC_GENPARSER := 1

ifdef LKC_GENPARSER

%.tab.c %.tab.h: $(srcdir)/%.y
	bison -t -d -v -b $* -p $(notdir $*) $<

lex.%.c: $(srcdir)/%.l
	flex -P$(notdir $*) -o$@ $<
else

lex.zconf.o: lex.zconf.c $(SHARED_DEPS)
	$(HOSTCC) $(HOSTCFLAGS) -I$(srcdir) -c $< -o $@

lex.zconf.c: $(srcdir)/lex.zconf.c_shipped
	cp $< $@

zconf.tab.c: $(srcdir)/zconf.tab.c_shipped
	cp $< $@

zconf.tab.h: $(srcdir)/zconf.tab.h_shipped
	cp $< $@
endif

zconf.tab.o: zconf.tab.c lex.zconf.c $(srcdir)/confdata.c $(srcdir)/expr.c \
             $(srcdir)/symbol.c $(srcdir)/menu.c $(SHARED_DEPS)
	$(HOSTCC) $(HOSTCFLAGS) -I$(srcdir) -I. -c $< -o $@

.PHONY: ncurses

ncurses:
	@echo "main() {}" > lxtemp.c
	@if $(HOSTCC) lxtemp.c $(LIBS) ; then \
		rm -f lxtemp.c a.out; \
	else \
		rm -f lxtemp.c; \
		echo -e "\007" ;\
		echo ">> Unable to find the Ncurses libraries." ;\
		echo ">>" ;\
		echo ">> You must have Ncurses installed in order" ;\
		echo ">> to use 'make menuconfig'" ;\
		echo ;\
		exit 1 ;\
	fi

clean:
	rm -f *.o *~ ../../*~ core *.exe $(TARGETS) $(MCONF_OBJS) $(CONF_OBJS)
	rm -f conf conf.exe mconf mconf.exe zconf.tab.c zconf.tab.h lex.zconf.c lkc_defs.h
	rm -f ../..config.h

