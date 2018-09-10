#
# Copyright (C) 2018 Jianhui Zhao <jianhuizhao329@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR := 0x20008000

define Device/Default
  PROFILES = Default
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | append-dtb | uImage none
endef

define Device/tq210
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS := TQ210
  DEVICE_TITLE := TQ210
endef
TARGET_DEVICES += tq210

$(eval $(call BuildImage))
