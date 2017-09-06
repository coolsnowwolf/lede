#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=osafeloader
PKG_RELEASE:=1

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/osafeloader
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Utility for handling TP-LINK SafeLoader images
  MAINTAINER:=Rafał Miłecki <rafal@milecki.pl>
  DEPENDS:=@TARGET_bcm53xx
endef

define Package/osafeloader/description
 This package contains an utility that allows handling SafeLoader images.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall"
endef

define Package/osafeloader/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/osafeloader $(1)/usr/bin/
endef

$(eval $(call BuildPackage,osafeloader))
