#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=soloscli
PKG_VERSION:=1.04
PKG_RELEASE:=1

PKG_SOURCE:=solos-pci-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/openadsl
PKG_HASH:=6379e6970a5c97fd5a223d024138ebb71b15d70e2ad1fe9da09edc5b2d760e1d
PKG_LICENSE:=GPL-2.0

PKG_BUILD_DIR:=$(BUILD_DIR)/solos-pci-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/soloscli
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Configuration utility for Solos ADSL2+ modems
  DEPENDS:=+kmod-solos-pci
  URL:=http://sourceforge.net/projects/openadsl
endef

define Package/soloscli/description
  This package contains the soloscli utility
  for interrogating Traverse Technologies' Solos ADSL2+ modems.
endef

define Package/soloscli/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/soloscli/soloscli $(1)/usr/bin/
	$(INSTALL_BIN) ./files/solos-log-stats $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/atm
	$(INSTALL_DATA) ./files/etc/hotplug.d/atm/15-solos-init $(1)/etc/hotplug.d/atm/
endef

$(eval $(call BuildPackage,soloscli))
