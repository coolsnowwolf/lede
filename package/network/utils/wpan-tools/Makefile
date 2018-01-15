#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wpan-tools
PKG_VERSION:=0.7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://wpan.cakelab.org/releases/
PKG_HASH:=c16de9d7861c2d9b6a4436a0fac730f9f545ee290b92bc770c538ec6a3f22309

include $(INCLUDE_DIR)/package.mk

define Package/wpan-tools
  SECTION:=net
  CATEGORY:=Network
  TITLE:=cfg802154 interface configuration utility
  URL:=http://wpan.cakelab.org/
  DEPENDS:= +libnl
endef

define Build/Configure
  $(call Build/Configure/Default)
endef

define Package/wpan-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/iwpan $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpan-ping/wpan-ping $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,wpan-tools))
