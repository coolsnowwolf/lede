#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hub-ctrl
PKG_VERSION:=1.0
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/codazoda/hub-ctrl.c.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=3eef0240fac90067d081551e9f73dfa741d94a9e
PKG_LICENSE:=GPL-2.0-or-later

PKG_MAINTAINER:=Simon Peter <probono@puredarwin.org>

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MIRROR_HASH:=98ffd9ead07d6208acf7f713dc6574877d4967bdfb47bf70444ec9d44399faaa
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/hub-ctrl
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Control USB power on a port by port basis 
  URL:=https://github.com/codazoda/hub-ctrl.c
  DEPENDS:=+libusb-compat
endef

define Package/hub-ctrl/description
  Control USB power on a port by port basis on some USB hubs.
  This only works on USB hubs that have the hardware necessary 
  to allow software controlled power switching.
  Most hubs DO NOT include the hardware.
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include \
		-L$(STAGING_DIR)/usr/lib -lusb \
		-o $(PKG_BUILD_DIR)/hub-ctrl $(PKG_BUILD_DIR)/hub-ctrl.c
endef

define Package/hub-ctrl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hub-ctrl $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,hub-ctrl))
