#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=om-watchdog
PKG_RELEASE:=3
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/om-watchdog
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=om watchdog
  URL:=http://openwrt.org/
endef

define Package/om-watchdog/description
 This package contains the hw watchdog script for the OM1P and OM2P device.
endef

define Build/Compile
endef

define Package/om-watchdog/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) ./files/om-watchdog.init $(1)/etc/init.d/om-watchdog
	$(INSTALL_BIN) ./files/om-watchdog $(1)/sbin/om-watchdog
endef

$(eval $(call BuildPackage,om-watchdog))
