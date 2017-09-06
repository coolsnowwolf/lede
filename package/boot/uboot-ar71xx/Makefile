#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2010.03
PKG_RELEASE:=1

PKG_HASH:=902d1b2c15787df55186fae4033685fb0c5a5a12755a08383e97c4a3e255925b

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=ar71xx
  BUILD_SUBTARGET:=generic
endef

define U-Boot/nbg460n_550n_550nh
  TITLE:=NBG460N/550N/550NH routers
  BUILD_DEVICES:=NBG_460N_550N_550NH
  HIDDEN:=y
endef

UBOOT_MAKE_FLAGS :=

UBOOT_TARGETS:=nbg460n_550n_550nh

$(eval $(call BuildPackage/U-Boot))
