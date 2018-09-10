#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME := loader
PKG_VERSION := 0.05

PKG_BUILD_DIR := $(KDIR)/$(PKG_NAME)-$(PKG_VERSION)$(LOADER_TYPE)

$(PKG_BUILD_DIR)/.prepared:
	mkdir $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	touch $@

$(PKG_BUILD_DIR)/lzma.elf: $(PKG_BUILD_DIR)/.prepared $(PKG_BUILD_DIR)/vmlinux.lzma
	PATH="$(TARGET_PATH)" $(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" CROSS_COMPILE="$(TARGET_CROSS)"

$(PKG_BUILD_DIR)/vmlinux.lzma: $(KDIR)/vmlinux.lzma
	$(CP) $< $@

$(KDIR)/loader$(LOADER_TYPE).elf: $(PKG_BUILD_DIR)/lzma.elf
	$(CP) $< $@

$(KDIR)/loader$(LOADER_TYPE).bin: $(PKG_BUILD_DIR)/lzma.bin
	$(CP) $< $@

download:
prepare: $(PKG_BUILD_DIR)/.prepared
compile: $(KDIR)/loader$(LOADER_TYPE).elf $(KDIR)/loader$(LOADER_TYPE).bin
install:

clean:
	rm -rf $(PKG_BUILD_DIR)
	rm -f $(KDIR)/loader.elf
	rm -f $(KDIR)/loader.bin
