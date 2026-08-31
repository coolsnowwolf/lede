# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2010 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR := 0x80100000
ROOTFS_PARTSIZE := 16M

define Device/Default
  PROFILES := Default
  KERNEL_NAME := vmlinux.elf
  KERNEL_INITRAMFS_NAME := vmlinux-initramfs.elf
  KERNEL := kernel-bin
  KERNEL_INSTALL := 1
  FILESYSTEMS := ext4 squashfs
  IMAGES := rootfs.img
  IMAGE/rootfs.img := append-rootfs | pad-to $(ROOTFS_PARTSIZE)
  IMAGE/rootfs.img.gz := append-rootfs | pad-to $(ROOTFS_PARTSIZE) | gzip
  ARTIFACTS := uImage-lzma uImage-gzip
  ARTIFACT/uImage-lzma := kernel-bin | lzma | uImage lzma
  ARTIFACT/uImage-gzip := kernel-bin | gzip | uImage gzip
  SUPPORTED_DEVICES :=
endef

define Device/generic
  DEVICE_VENDOR := MIPS
  DEVICE_MODEL := Malta CoreLV board (QEMU)
endef
TARGET_DEVICES += generic

$(eval $(call BuildImage))
