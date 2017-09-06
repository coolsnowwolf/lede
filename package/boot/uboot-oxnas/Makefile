#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2014.10
PKG_RELEASE:=1

PKG_HASH:=d3b132a7a9b3f3182b7aad71c2dfbd4fc15bea83e12c76134eb3ffefc07d1c71

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=oxnas
  BUILD_DEVICES:=Default
  HIDDEN:=y
endef

define U-Boot/ox820
  NAME:=Oxford/PLX NAS7820
endef

UBOOT_TARGETS:=ox820

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/u-boot.bin $(STAGING_DIR_IMAGE)/u-boot.bin
endef

$(eval $(call BuildPackage/U-Boot))
