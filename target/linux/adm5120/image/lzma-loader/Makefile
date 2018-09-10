#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LOADER		:= loader.bin
LOADER_NAME	:= $(basename $(notdir $(LOADER)))
LOADER_DATA 	:=
TARGET_DIR	:=

ifeq ($(TARGET_DIR),)
TARGET_DIR	:= $(KDIR)
endif

LOADER_BIN	:= $(TARGET_DIR)/$(LOADER_NAME).bin
LOADER_GZ	:= $(TARGET_DIR)/$(LOADER_NAME).gz
LOADER_ELF	:= $(TARGET_DIR)/$(LOADER_NAME).elf

LZMA_STARTUP_ORG:= 0
LZMA_TEXT_START	:= 0x80300000

PKG_NAME := lzma-loader
PKG_BUILD_DIR := $(KDIR)/$(PKG_NAME)

.PHONY : loader-compile loader.bin loader.elf loader.gz

$(PKG_BUILD_DIR)/.prepared:
	mkdir $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	touch $@

loader-compile: $(PKG_BUILD_DIR)/.prepared
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE="$(TARGET_CROSS)" \
		LZMA_STARTUP_ORG=$(LZMA_STARTUP_ORG) \
		LZMA_TEXT_START=$(LZMA_TEXT_START) \
		LOADER_DATA=$(LOADER_DATA) \
		CONFIG_BOARD=$(CONFIG_BOARD) \
		CONFIG_PASS_KARGS=$(CONFIG_PASS_KARGS) \
		clean all

loader.gz: $(PKG_BUILD_DIR)/loader.bin
	gzip -nc9 $< > $(LOADER_GZ)

loader.elf: $(PKG_BUILD_DIR)/loader.elf
	$(CP) $< $(LOADER_ELF)

loader.bin: $(PKG_BUILD_DIR)/loader.bin
	$(CP) $< $(LOADER_BIN)

download:
prepare: $(PKG_BUILD_DIR)/.prepared
compile: loader-compile

install:

clean:
	rm -rf $(PKG_BUILD_DIR)

