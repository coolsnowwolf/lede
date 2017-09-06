#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=am33x-cm3
PKG_VERSION:=20130304
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=http://arago-project.org/git/projects/am33x-cm3.git
PKG_SOURCE_VERSION:=32cf44e25b5828b87af6dceebc3a49fed5d858ac
PKG_MIRROR_HASH:=71d4029eed4ef58a1e0beb299d5c876bf0b9a790ecac52aacf1d510f5841d959

PKG_MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/am33x-cm3
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=http://arago-project.org/git/projects/?p=am33x-cm3.git
  TITLE:=TI AM335x PM firmware
  DEPENDS:=@TARGET_omap
endef

define Build/Compile
	$(MAKE) CROSS_COMPILE=$(TARGET_CROSS) -C $(PKG_BUILD_DIR)
endef

define Package/am33x-cm3/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(CP) $(PKG_BUILD_DIR)/bin/am335x-pm-firmware.bin $(1)/lib/firmware
endef

$(eval $(call BuildPackage,am33x-cm3))
