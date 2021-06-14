#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=maccalc
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/maccalc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=MAC address calculation
endef

define Package/maccalc/description
 This package contains a MAC address manipulation utility.
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/maccalc/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/maccalc $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,maccalc))
