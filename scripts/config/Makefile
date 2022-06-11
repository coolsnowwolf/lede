# SPDX-License-Identifier: GPL-2.0-only
# ===========================================================================
# OpenWrt configuration targets

.PHONY: clean all
all: conf mconf
clean:
	rm -f *.o lxdialog/*.o *.moc $(clean-files) conf mconf qconf nconf

# This clean-files definition is here to ensure that temporary files from the
# previous version are removed by make config-clean.
# It should be removed or emptied when this Makefile get updated again.
clean-files	:= zconf.tab.c zconf.lex.c zconf.hash.c .tmp_qtcheck

# ===========================================================================
# Variables needed by the upstream Makefile

# Avoids displaying 'UPD mconf-cfg' in an otherwise quiet make menuconfig
kecho:=true

CONFIG_SHELL:=$(SHELL)
srctree:=.
src:=.
obj:=.
Q:=$(if $V,,@)
cmd = $(cmd_$(1))
dot-target = $(dir $@).$(notdir $@)

# taken from ../Kbuild.include
define filechk
	$(Q)set -e;						\
	mkdir -p $(dir $@);					\
	trap "rm -f $(dot-target).tmp" EXIT;			\
	{ $(filechk_$(1)); } > $(dot-target).tmp;		\
	if [ ! -r $@ ] || ! cmp -s $@ $(dot-target).tmp; then	\
		$(kecho) '  UPD     $@';			\
		mv -f $(dot-target).tmp $@;			\
	fi
endef

### Stripped down upstream Makefile follows:
# ===========================================================================
# object files used by all kconfig flavours
common-objs	:= confdata.o expr.o lexer.lex.o parser.tab.o preprocess.o \
		   symbol.o util.o

$(obj)/lexer.lex.o: $(obj)/parser.tab.h
HOSTCFLAGS_lexer.lex.o	:= -I $(srctree)/$(src)
HOSTCFLAGS_parser.tab.o	:= -I $(srctree)/$(src)

# conf: Used for defconfig, oldconfig and related targets
hostprogs-y	+= conf
conf-objs	:= conf.o $(common-objs)

# nconf: Used for the nconfig target based on ncurses
hostprogs-y	+= nconf
nconf-objs	:= nconf.o nconf.gui.o $(common-objs)

HOSTLDLIBS_nconf	= $(shell . $(obj)/nconf-cfg && echo $$libs)
HOSTCFLAGS_nconf.o	= $(shell . $(obj)/nconf-cfg && echo $$cflags)
HOSTCFLAGS_nconf.gui.o	= $(shell . $(obj)/nconf-cfg && echo $$cflags)

$(obj)/nconf.o $(obj)/nconf.gui.o: $(obj)/nconf-cfg

# mconf: Used for the menuconfig target based on lxdialog
hostprogs-y	+= mconf
lxdialog	:= $(addprefix lxdialog/, \
		     checklist.o inputbox.o menubox.o textbox.o util.o yesno.o)
mconf-objs	:= mconf.o $(lxdialog) $(common-objs)

HOSTLDLIBS_mconf = $(shell . $(obj)/mconf-cfg && echo $$libs)
$(foreach f, mconf.o $(lxdialog), \
  $(eval HOSTCFLAGS_$f = $$(shell . $(obj)/mconf-cfg && echo $$$$cflags)))

$(addprefix $(obj)/, mconf.o $(lxdialog)): $(obj)/mconf-cfg

# qconf: Used for the xconfig target based on Qt
hostprogs-y	+= qconf
qconf-cxxobjs	:= qconf.o
qconf-objs	:= images.o $(common-objs)

HOSTLDLIBS_qconf	= $(shell . $(obj)/qconf-cfg && echo $$libs)
HOSTCXXFLAGS_qconf.o	= $(shell . $(obj)/qconf-cfg && echo $$cflags)

$(obj)/qconf.o: $(obj)/qconf-cfg $(obj)/qconf.moc

quiet_cmd_moc = MOC     $@
      cmd_moc = $(shell . $(obj)/qconf-cfg && echo $$moc) -i $< -o $@

$(obj)/%.moc: $(src)/%.h $(obj)/qconf-cfg
	$(call cmd,moc)

# check if necessary packages are available, and configure build flags
filechk_conf_cfg = $(CONFIG_SHELL) $<

$(obj)/%conf-cfg: $(src)/%conf-cfg.sh FORCE
	$(call filechk,conf_cfg)

clean-files += *conf-cfg

# ===========================================================================
# OpenWrt rules and final adjustments that need to be made after reading the
# full upstream Makefile

FORCE:

ifdef BUILD_SHIPPED_FILES
shipped-files := lexer.lex.c parser.tab.c parser.tab.h
clean-files += $(shipped-files)

.SECONDARY: $(shipped-files)

%.tab.c %.tab.h: %.y
	bison -l -d -b $* $<

%.lex.c: %.l
	flex -L -o$@ $<
endif

$(foreach f, mconf.o $(lxdialog), \
  $(eval $f: CFLAGS+=$$(HOSTCFLAGS_$f)))

$(obj)/lexer.lex.o: CFLAGS += $(HOSTCFLAGS_lexer.lex.o)
$(obj)/parser.tab.o: CFLAGS += $(HOSTCFLAGS_parser.tab.o)
$(obj)/qconf.o: CXXFLAGS+=$(HOSTCXXFLAGS_qconf.o)

conf: $(conf-objs)

# The *conf-cfg file is used (then filtered out) as the first prerequisite to
# avoid sourcing it before the script is built, when trying to compute CFLAGS
# for the actual first prerequisite.  This avoids errors like:
# '/bin/sh: ./mconf-cfg: No such file or directory'
mconf: mconf-cfg $(mconf-objs)
	$(CC) -o $@ $(filter-out mconf-cfg,$^) $(HOSTLDLIBS_mconf)

nconf: nconf-cfg $(nconf-objs)
	$(CC) -o $@ $(filter-out nconf-cfg,$^) $(HOSTLDLIBS_nconf)

qconf: qconf-cfg $(qconf-cxxobjs) $(qconf-objs)
	$(CXX) -o $@ $(filter-out qconf-cfg,$^) $(HOSTLDLIBS_qconf)
