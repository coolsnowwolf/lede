#
# Copyright (C) 2020 Brian Norris <computersforpeace@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bottlerocket
PKG_VERSION:=0.04c
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://linuxha.com/bottlerocket
PKG_HASH:=deb8fbf8856f87af15fa9883e07b1fddb2aa78f6b4c7d119ae4bd664dc1d19ae

PKG_MAINTAINER:=Brian Norris <computersforpeace@gmail.com>
PKG_LICENSE:=LGPL-2.1-only

include $(INCLUDE_DIR)/package.mk

define Package/bottlerocket
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Bottlerocket
  URL:=https://linuxha.com/bottlerocket/
endef

define Package/bottlerocket/description
  BottleRocket is an interface to the X10 FireCracker home automation kit.
endef

define Package/bottlerocket/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/br $(1)/usr/bin/
endef

$(eval $(call BuildPackage,bottlerocket))
