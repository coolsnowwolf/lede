# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR := 0x80100000

define Device/Default
  PROFILES := Default
  FILESYSTEMS := squashfs
  KERNEL_DEPENDS = $$(wildcard $$(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  KERNEL_SUFFIX := -kernel.itb
  KERNEL_INSTALL := 1
  KERNEL_IN_UBI := 1
  DEVICE_DTS_CONFIG := config@1
  IMAGES := factory.ubi sysupgrade.tar
  IMAGE/factory.ubi := append-ubi
  IMAGE/sysupgrade.tar := sysupgrade-tar
endef

define Device/img_creator-ci40
  DEVICE_VENDOR := Imagination Technologies
  DEVICE_MODEL := Creator Ci40 (VL-62899)
  DEVICE_ALT0_VENDOR := Imagination Technologies
  DEVICE_ALT0_MODEL := Marduk board
  BOARD_NAME := img,pistachio-marduk
  DEVICE_DTS := img/pistachio_marduk
  BLOCKSIZE := 256KiB
  PAGESIZE := 4KiB
  DEVICE_PACKAGES := kmod-tpm-i2c-infineon kmod-ca8210 wpan-tools
endef

TARGET_DEVICES += img_creator-ci40


$(eval $(call BuildImage))
