#
# Copyright (C) 2015 Rafał Miłecki <zajec5@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=otrx
PKG_RELEASE:=1

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/otrx
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Utility for opening (analyzing) TRX firmware images
  MAINTAINER:=Rafał Miłecki <zajec5@gmail.com>
  DEPENDS:=@(TARGET_brcm47xx||TARGET_bcm53xx)
endef

define Package/otrx/description
 This package contains an utility that allows validating TRX images.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall"
endef

define Package/otrx/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/otrx $(1)/usr/bin/
endef

$(eval $(call BuildPackage,otrx))
