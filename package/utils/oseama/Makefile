#
# Copyright (C) 2016 Rafał Miłecki <zajec5@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=oseama
PKG_RELEASE:=1

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/oseama
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Utility for handling Seama firmware images
  MAINTAINER:=Rafał Miłecki <zajec5@gmail.com>
  DEPENDS:=@TARGET_bcm53xx
endef

define Package/oseama/description
 This package contains an utility that allows handling Seama images.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall"
endef

define Package/oseama/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oseama $(1)/usr/bin/
endef

$(eval $(call BuildPackage,oseama))
