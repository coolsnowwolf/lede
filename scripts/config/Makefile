# SPDX-License-Identifier: GPL-2.0-only
# ===========================================================================
# OpenWrt configuration targets

.PHONY: clean all
all: conf mconf
clean:
	rm -f *.o lxdialog/*.o *.moc .*.cmd $(clean-files)

# This clean-files definition is here to ensure that temporary files from the
# previous version are removed by make config-clean.
# It should be emptied after the end of support for OpenWrt 19.07.
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

# some definitions taken from ../Kbuild.include
dot-target = $(dir $@).$(notdir $@)
squote  := '
escsq = $(subst $(squote),'\$(squote)',$1)
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
cmd-check = $(if $(strip $(cmd_$@)),,1)
make-cmd = $(call escsq,$(subst $(pound),$$(pound),$(subst $$,$$$$,$(cmd_$(1)))))
newer-prereqs = $(filter-out $(PHONY),$?)
if_changed = $(if $(newer-prereqs)$(cmd-check),			\
	$(cmd);							\
	printf '%s\n' 'cmd_$@ := $(make-cmd)' > $(dot-target).cmd, @:)

### Stripped down upstream Makefile follows:
# ===========================================================================
# object files used by all kconfig flavours
common-objs	:= confdata.o expr.o lexer.lex.o menu.o parser.tab.o \
		   preprocess.o symbol.o util.o

$(obj)/lexer.lex.o: $(obj)/parser.tab.h
HOSTCFLAGS_lexer.lex.o	:= -I $(srctree)/$(src)
HOSTCFLAGS_parser.tab.o	:= -I $(srctree)/$(src)

# conf: Used for defconfig, oldconfig and related targets
hostprogs	+= conf
conf-objs	:= conf.o $(common-objs)

# nconf: Used for the nconfig target based on ncurses
hostprogs	+= nconf
nconf-objs	:= nconf.o nconf.gui.o $(common-objs)

HOSTLDLIBS_nconf	= $(shell . $(obj)/nconf-cfg && echo $$libs)
HOSTCFLAGS_nconf.o	= $(shell . $(obj)/nconf-cfg && echo $$cflags)
HOSTCFLAGS_nconf.gui.o	= $(shell . $(obj)/nconf-cfg && echo $$cflags)

$(obj)/nconf.o $(obj)/nconf.gui.o: $(obj)/nconf-cfg

# mconf: Used for the menuconfig target based on lxdialog
hostprogs	+= mconf
lxdialog	:= $(addprefix lxdialog/, \
		     checklist.o inputbox.o menubox.o textbox.o util.o yesno.o)
mconf-objs	:= mconf.o $(lxdialog) $(common-objs)

HOSTLDLIBS_mconf = $(shell . $(obj)/mconf-cfg && echo $$libs)
$(foreach f, mconf.o $(lxdialog), \
  $(eval HOSTCFLAGS_$f = $$(shell . $(obj)/mconf-cfg && echo $$$$cflags)))

$(addprefix $(obj)/, mconf.o $(lxdialog)): $(obj)/mconf-cfg

# qconf: Used for the xconfig target based on Qt
hostprogs	+= qconf
qconf-cxxobjs	:= qconf.o qconf-moc.o
qconf-objs	:= images.o $(common-objs)

HOSTLDLIBS_qconf	= $(shell . $(obj)/qconf-cfg && echo $$libs)
HOSTCXXFLAGS_qconf.o	= $(shell . $(obj)/qconf-cfg && echo $$cflags)
HOSTCXXFLAGS_qconf-moc.o = $(shell . $(obj)/qconf-cfg && echo $$cflags)

$(obj)/qconf.o: $(obj)/qconf-cfg

quiet_cmd_moc = MOC     $@
      cmd_moc = $(shell . $(obj)/qconf-cfg && echo $$moc) $< -o $@

$(obj)/qconf-moc.cc: $(src)/qconf.h $(obj)/qconf-cfg FORCE
	$(call if_changed,moc)

targets += qconf-moc.cc

# check if necessary packages are available, and configure build flags
filechk_conf_cfg = $(CONFIG_SHELL) $<

$(obj)/%conf-cfg: $(src)/%conf-cfg.sh FORCE
	$(call filechk,conf_cfg)

clean-files += *conf-cfg

# ===========================================================================
# OpenWrt rules and final adjustments that need to be made after reading the
# full upstream Makefile

clean-files += $(targets) $(hostprogs)

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

$(foreach f,$(conf-objs) $(filter-out $(common-objs),$(mconf-objs) \
						     $(qconf-objs) \
						     $(nconf-objs)), \
  $(eval $(obj)/$f: CFLAGS+=$$(HOSTCFLAGS_$f)))

$(foreach f,$(qconf-cxxobjs), \
  $(eval $(obj)/$f: CXXFLAGS+=$$(HOSTCXXFLAGS_$f)))

$(obj)/conf: $(addprefix $(obj)/,$(conf-objs))

# The *conf-cfg file is used (then filtered out) as the first prerequisite to
# avoid sourcing it before the script is built, when trying to compute CFLAGS
# for the actual first prerequisite.  This avoids errors like:
# '/bin/sh: ./mconf-cfg: No such file or directory'
$(obj)/mconf: mconf-cfg $(addprefix $(obj)/,$(mconf-objs))
	$(CC) -o $@ $(filter-out mconf-cfg,$^) $(HOSTLDLIBS_mconf)

$(obj)/nconf: nconf-cfg $(addprefix $(obj)/,$(nconf-objs))
	$(CC) -o $@ $(filter-out nconf-cfg,$^) $(HOSTLDLIBS_nconf)

$(obj)/qconf: qconf-cfg $(addprefix $(obj)/,$(qconf-cxxobjs) $(qconf-objs))
	$(CXX) -o $@ $(filter-out qconf-cfg,$^) $(HOSTLDLIBS_qconf)
