# Makefile for OpenWrt
#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

TOPDIR:=${CURDIR}
LC_ALL:=C
LANG:=C
SDK:=1
export TOPDIR LC_ALL LANG SDK

world:

DISTRO_PKG_CONFIG:=$(shell which -a pkg-config | grep -E '\/usr' | head -n 1)
export PATH:=$(TOPDIR)/staging_dir/host/bin:$(PATH)

ifneq ($(OPENWRT_BUILD),1)
  override OPENWRT_BUILD=1
  export OPENWRT_BUILD

  empty:=
  space:= $(empty) $(empty)
  _SINGLE=export MAKEFLAGS=$(space);

  include $(TOPDIR)/include/debug.mk
  include $(TOPDIR)/include/depends.mk
  include $(TOPDIR)/include/toplevel.mk
else
  include rules.mk
  include $(INCLUDE_DIR)/depends.mk
  include $(INCLUDE_DIR)/subdir.mk
  include package/Makefile

$(package/stamp-compile): $(BUILD_DIR)/.prepared
$(BUILD_DIR)/.prepared: Makefile
	@mkdir -p $$(dirname $@)
	@touch $@

clean: FORCE
	git clean -f -d $(STAGING_DIR); true
	git clean -f -d $(BUILD_DIR); true
	git clean -f -d $(BIN_DIR); true

dirclean: clean
	git reset --hard HEAD
	git clean -f -d
	rm -rf feeds/

# check prerequisites before starting to build
prereq: $(package/stamp-prereq) ;

world: prepare $(package/stamp-compile) FORCE
	@$(MAKE) package/index

.PHONY: clean dirclean prereq prepare world

endif
