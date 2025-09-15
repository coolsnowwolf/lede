# SPDX-License-Identifier: GPL-2.0-or-later

#
# Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
# Copyright (C) 2014 Jonas Gorski <jogo@openwrt.org>
# Copyright (C) 2011 OpenWrt.org
# Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
#

include $(TOPDIR)/rules.mk

LZMA_TEXT_START	:= 0x80a00000
LOADER		:= loader.bin
LOADER_NAME	:= $(basename $(notdir $(LOADER)))
LOADER_DATA 	:=
TARGET_DIR	:=

UART_BASE_3329 := 0xb0000100
UART_BASE_3368 := 0xfff8c100
UART_BASE_3380 := 0xb4e00200
UART_BASE_3383 := 0xb4e00500
UART_BASE_3384 := 0xb4e00500
UART_BASE_6318 := 0xb0000100
UART_BASE_6328 := 0xb0000100
UART_BASE_6338 := 0xfffe0300
UART_BASE_6345 := 0xfffe0300
UART_BASE_6348 := 0xfffe0300
UART_BASE_6358 := 0xfffe0100
UART_BASE_6362 := 0xb0000100
UART_BASE_6368 := 0xb0000100
UART_BASE_63268 := 0xb0000180
UART_BASE_6816 := 0xb0000100
UART_BASE_6818 := 0xb0000100
UART_BASE_6828 := 0xb0000180
UART_BASE := $(if $(UART_BASE_$(CHIP_ID)),$(UART_BASE_$(CHIP_ID)),0)

ifeq ($(TARGET_DIR),)
TARGET_DIR	:= $(KDIR)
endif

LOADER_BIN	:= $(TARGET_DIR)/$(LOADER_NAME).bin
LOADER_ELF	:= $(TARGET_DIR)/$(LOADER_NAME).elf

PKG_NAME := lzma-loader
PKG_BUILD_DIR := $(KDIR)/$(PKG_NAME)

.PHONY : loader-compile loader.bin loader.elf

$(PKG_BUILD_DIR)/.prepared:
	mkdir $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	touch $@

loader-compile: $(PKG_BUILD_DIR)/.prepared
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE="$(TARGET_CROSS)" \
		LZMA_TEXT_START=$(LZMA_TEXT_START) \
		LOADER_DATA=$(LOADER_DATA) \
		UART_BASE=$(UART_BASE) \
		clean all

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
