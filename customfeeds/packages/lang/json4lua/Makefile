#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#  

include $(TOPDIR)/rules.mk

PKG_NAME:=json4lua
PKG_VERSION:=0.9.54
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/amrhassan/json4lua/tar.gz/$(PKG_VERSION)?
PKG_HASH:=2498e5224b024521af7f2de9a2c2af4028cd8da4b724ffd6bd94beede2402cfe

PKG_MAINTAINER:=Amr Hassan <amr.hassan@gmail.com>
PKG_LICENSE:=MIT

LUA_MODULE_PATH:=/usr/lib/lua

include $(INCLUDE_DIR)/package.mk

define Package/json4lua
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=json4lua
  URL:=https://github.com/amrhassan/json4lua
  DEPENDS:=+lua +luasocket
  PKGARCH:=all
endef

define Package/json4lua/description
	JSON and JSONRPC for Lua
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/json4lua/install
	$(INSTALL_DIR) $(1)/$(LUA_MODULE_PATH)/json
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/json/json.lua $(1)/$(LUA_MODULE_PATH)/json.lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/json/rpc.lua $(1)/$(LUA_MODULE_PATH)/json/rpc.lua
endef

$(eval $(call BuildPackage,json4lua))
