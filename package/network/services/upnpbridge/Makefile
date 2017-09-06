#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=upnpbridge
PKG_RELEASE=1

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/upnpbridge
  SECTION:=net
  CATEGORY:=Network
  TITLE:=OpenWrt UPNP Multicast Bridge
  DEPENDS:=+USE_GLIBC:librt
endef

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/upnpbridge/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/upnpbridge $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/upnpbridge.init $(1)/etc/init.d/upnpbridge
endef

$(eval $(call BuildPackage,upnpbridge))
