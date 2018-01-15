#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2017.03
PKG_RELEASE:=1

PKG_HASH:=f54baf3f9325bf444c7905f3a5b6f83680edb1e6e1a4d5f8a5ad80abe885113f

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

UBOOT_MAKE_FLAGS:=

define U-Boot/Default
  BUILD_TARGET:=mvebu
  HIDDEN:=1
endef

define U-Boot/clearfog
  NAME:=SolidRun ClearFog A1
  BUILD_DEVICES:=armada-388-clearfog-base armada-388-clearfog-pro
  UBOOT_IMAGE:=u-boot-spl.kwb
endef

UBOOT_TARGETS:= \
	clearfog

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot-spl.kwb
endef

$(eval $(call BuildPackage/U-Boot))
