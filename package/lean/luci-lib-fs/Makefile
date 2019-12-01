#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-lib-fs
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/luci-lib-fs
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=luci-lib-fs
  PKGARCH:=all
  URL:=https://github.com/lbthomsen/openwrt-luci
  DEPENDS:=+luci +luci-lib-nixio
endef

define Package/luci-lib-fs/description
  luci-lib-fs
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/Install
endef


define Package/luci-lib-fs/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	$(CP) ./files/*.lua $(1)/usr/lib/lua/luci
	
endef

$(eval $(call BuildPackage,luci-lib-fs))
