#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR := 0x80400000

define Device/Default
  PROFILES := Default
  FILESYSTEMS := squashfs
  KERNEL_DEPENDS = $$(wildcard $$(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  KERNEL_SUFFIX := -kernel.itb
  KERNEL_INSTALL := 1
  KERNEL_IN_UBI := 1
  IMAGES := factory.ubi sysupgrade.tar
  IMAGE/factory.ubi := append-ubi
  IMAGE/sysupgrade.tar := sysupgrade-tar
endef

define Device/marduk
  BOARD_NAME := img,pistachio-marduk
  DEVICE_DTS := img/pistachio_marduk
  BLOCKSIZE := 256KiB
  PAGESIZE := 4KiB
  DEVICE_TITLE := Creator Ci40
  DEVICE_PACKAGES := kmod-tpm-i2c-infineon
endef

TARGET_DEVICES += marduk


$(eval $(call BuildImage))
