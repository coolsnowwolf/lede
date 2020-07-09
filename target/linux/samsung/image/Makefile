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
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $$(SOC)_$(1)
endef

ifeq ($(SUBTARGET),s5pv210)

define Device/embedsky_tq210
  DEVICE_VENDOR := EmbedSky
  DEVICE_MODEL := TQ210
  SOC := s5pv210
endef
TARGET_DEVICES += embedsky_tq210

endif

$(eval $(call BuildImage))
