#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR := 0x20008000

ifeq ($(SUBTARGET),legacy)
include ./legacy.mk
UBIFS_OPTS := -m 2048 -e 126KiB -c 2048
endif
ifeq ($(SUBTARGET),sama5d3)
include ./sama5d3.mk
UBIFS_OPTS := -m 2048 -e 124KiB -c 2048
endif

define Device/Default
  PROFILES := Default
  FILESYSTEMS := squashfs ubifs
  DEVICE_DTS := $(1)
  KERNEL_NAME := zImage
  KERNEL_SIZE := 4096k
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  DTB_SIZE :=
endef

define Device/dtb
  KERNEL := kernel-bin | lzma | uImage lzma
endef

define Device/evaluation
  KERNEL_INSTALL := 1
  KERNEL_SUFFIX := -uImage
  IMAGES := root.ubi
  IMAGE/root.ubi := append-ubi
endef

define Device/evaluation-dtb
  $(Device/evaluation)
  $(Device/dtb)
  IMAGES += dtb
  IMAGE/dtb := install-dtb
endef

define Device/production
  UBINIZE_OPTS := -E 5
  IMAGES := factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
endef

define Device/production-dtb
  $(Device/production)
  $(Device/dtb)
  DTB_SIZE := 128k
  IMAGE/factory.bin := append-dtb | pad-to $$$$(DTB_SIZE) \
	  | append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
endef

$(eval $(call BuildImage))
