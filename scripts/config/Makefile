# ===========================================================================
# OpenWrt configuration targets
# These targets are used from top-level makefile

# ===========================================================================
# Shared Makefile for the various kconfig executables:
# conf:	  Used for defconfig, oldconfig and related targets
# mconf:  Used for the mconfig target.
#         Utilizes the lxdialog package
# object files used by all kconfig flavours


# Platform specific fixes
#
# FreeBSD

check_lxdialog = $(shell $(SHELL) $(CURDIR)/lxdialog/check-lxdialog.sh -$(1))
export CFLAGS += -DKBUILD_NO_NLS -I. $(call check_lxdialog,ccflags)

conf-objs	:= conf.o zconf.tab.o
mconf-objs	:= mconf.o zconf.tab.o
lxdialog-objs := \
	lxdialog/checklist.o lxdialog/util.o lxdialog/inputbox.o \
	lxdialog/textbox.o lxdialog/yesno.o lxdialog/menubox.o

clean-files	:= zconf.tab.c lex.zconf.c zconf.hash.c

all: conf mconf

conf: $(conf-objs)
mconf: $(mconf-objs) $(lxdialog-objs)
	$(CC) -o $@ $^ $(call check_lxdialog,ldflags $(CC))

clean:
	rm -f *.o lxdialog/*.o $(clean-files) conf mconf

zconf.tab.o: zconf.lex.c zconf.hash.c confdata.c

kconfig_load.o: lkc_defs.h

zconf.tab.c: zconf.y
zconf.lex.c: zconf.l
zconf.hash.c: zconf.gperf

%.tab.c: %.y
	cp $@_shipped $@ || bison -l -b $* -p $(notdir $*) $<

%.lex.c: %.l
	cp $@_shipped $@ || flex -L -P$(notdir $*) -o$@ $<

%.hash.c: %.gperf
	cp $@_shipped $@ || gperf < $< > $@
