#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=yamonenv
PKG_VERSION:=20051022
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_gruen.4g__$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.openwrt.org/sources/
PKG_HASH:=466eca9cdad2c15e957fb9ce7d0b6927ecd17d85c4cc2dff37e97a3e6b209c67

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/yamonenv
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Boot Loaders
  DEPENDS:=@TARGET_au1000
  TITLE:=YAMON configuration utility
  URL:=http://meshcube.org/nylon/stable/sources/
  MAINTAINER:=Florian Fainelli <florian@openwrt.org>
endef

define Build/Configure
endef

define Package/yamonenv/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,yamonenv))
