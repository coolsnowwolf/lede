#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

LOADADDR = 0x81000000		# RAM start + 16M
KERNEL_ENTRY = 0x80101000
RAMSIZE = 0x00100000		# 1MB
IMAGE_COPY = 1

LOADER_MAKEOPTS= \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		PATH="$(TARGET_PATH)" \
		CC="$(TARGET_CC)" \
		KDIR=$(KDIR) \
		LOADADDR=$(LOADADDR) \
		KERNEL_ENTRY=$(KERNEL_ENTRY) \
		RAMSIZE=$(RAMSIZE) \
		IMAGE_COPY=$(IMAGE_COPY)


LOADER_DIR = $(GENERIC_PLATFORM_DIR)/image/lzma-loader

define Build/lzma-loader-elf
	rm -rf $@.loader
	mkdir -p $@.loader
	$(CP) $(LOADER_DIR)/src/* $@.loader/
	$(CP) $@ $@.loader/vmlinux.lzma
	$(MAKE) -C $@.loader $(LOADER_MAKEOPTS)
	cp $@.loader/lzma.elf $@
	rm -rf $@.loader
endef

define Device/nand
  CMDLINE := ubi.mtd=1 ubi.block=0,rootfs root=/dev/ubiblock0_1
  BOARD_NAME := rb532
  SUPPORTED_DEVICES := rb532
  DEVICE_TITLE := rb532 NAND
  KERNEL_INITRAMFS := append-kernel | patch-cmdline | lzma | lzma-loader-elf
  KERNEL := $$(KERNEL_INITRAMFS) | kernel2minor -s 2048 -i 0 -c
  IMAGES := sysupgrade.bin
  FILESYSTEMS := squashfs
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

TARGET_DEVICES := nand

define Build/Clean
	$(MAKE) -C $(GENERIC_PLATFORM_DIR)/image/lzma-loader $(LOADER_MAKEOPTS) clean
endef

define Image/Prepare
	cat $(KDIR)/vmlinux | $(STAGING_DIR_HOST)/bin/lzma e -si -so -eos -lc1 -lp2 -pb2 > $(KDIR)/vmlinux.lzma
	$(MAKE) -C $(GENERIC_PLATFORM_DIR)/image/lzma-loader $(LOADER_MAKEOPTS) clean compile
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR)/vmlinux-initramfs '$(strip $(call Image/cmdline/yaffs2)) '
	cat $(KDIR)/vmlinux-initramfs | $(STAGING_DIR_HOST)/bin/lzma e -si -so -eos -lc1 -lp2 -pb2 > $(KDIR)/vmlinux.lzma
	$(MAKE) -C $(GENERIC_PLATFORM_DIR)/image/lzma-loader $(LOADER_MAKEOPTS) clean compile
	$(CP) $(KDIR)/loader.elf $(KDIR)/loader-initramfs.elf
endif
endef

VMLINUX:=$(BIN_DIR)/$(IMG_PREFIX)-vmlinux-kernel

define Image/BuildKernel
	$(CP) $(KDIR)/loader.elf $(VMLINUX)
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
	$(CP) $(KDIR)/loader-initramfs.elf $(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs
endif
endef

define Image/cmdline/yaffs2
root=/dev/mtdblock1 rootfstype=yaffs2
endef

define Image/Build/squashfs
	dd if=/dev/zero bs=128k count=1 >> $(KDIR)/root.squashfs
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	$(CP) $(KDIR)/vmlinux.elf $(BIN_DIR)/$(IMG_PREFIX)-$(1).kernel
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(BIN_DIR)/$(IMG_PREFIX)-$(1).kernel 'root=/dev/sda2 '
	./gen_image.sh $(BIN_DIR)/$(IMG_PREFIX)-combined-$(1).bin \
		6 $(BIN_DIR)/$(IMG_PREFIX)-$(1).kernel \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(KDIR)/root.$(1) \
		128
endef

$(eval $(call BuildImage))
