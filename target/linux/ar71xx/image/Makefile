#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80060000

DEVICE_VARS += BOARDNAME CMDLINE CONSOLE IMAGE_SIZE LOADER_TYPE

ifeq ($(SUBTARGET),generic)
include ./generic.mk
include ./generic-legacy-devices.mk
include ./generic-senao.mk
include ./generic-tp-link.mk
include ./generic-ubnt.mk
endif
ifeq ($(SUBTARGET),tiny)
include ./tiny.mk
include ./tiny-tp-link.mk
include ./tiny-legacy-devices.mk
endif
ifeq ($(SUBTARGET),nand)
include ./nand.mk
endif
ifeq ($(SUBTARGET),mikrotik)
include ./mikrotik.mk
endif
include ./legacy.mk

define Build/loader-common
	rm -rf $@.src
	$(MAKE) -C lzma-loader \
		PKG_BUILD_DIR="$@.src" \
		TARGET_DIR="$(dir $@)" LOADER_NAME="$(notdir $@)" \
		BOARD="$(BOARDNAME)" \
		LZMA_TEXT_START=0x80a00000 LOADADDR=0x80060000 \
		$(1) compile loader.$(LOADER_TYPE)
	mv "$@.$(LOADER_TYPE)" "$@"
	rm -rf $@.src
endef

define Build/loader-kernel
	$(call Build/loader-common,LOADER_DATA="$@")
endef

define Build/loader-kernel-cmdline
	$(call Build/loader-common,LOADER_DATA="$@" KERNEL_CMDLINE="$(CMDLINE)")
endef

define Device/Default
  BOARDNAME :=
  DEVICE_PROFILE = $$(BOARDNAME)
  PROFILES = Default Minimal $$(DEVICE_PROFILE)
  MTDPARTS :=
  BLOCKSIZE := 64k
  CONSOLE := ttyS0,115200
  CMDLINE = $$(if $$(BOARDNAME),board=$$(BOARDNAME)) $$(if $$(MTDPARTS),mtdparts=$$(MTDPARTS)) $$(if $$(CONSOLE),console=$$(CONSOLE))
  KERNEL := kernel-bin | patch-cmdline | lzma | uImage lzma
  COMPILE :=
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin = append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
endef

$(eval $(call BuildImage))
