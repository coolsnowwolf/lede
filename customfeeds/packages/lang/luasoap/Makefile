#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luasoap
PKG_VERSION:=2014-08-21
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tomasguisasola/luasoap.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=af1e100281cee4b972df10121e37e51d53367a98
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=839317e9a12c0723cf15eb5cea8d249e3f666a077585018fb8757aa1fb47d4e8

PKG_MAINTAINER:=Liu Peng <pengliu@credosemi.com>
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/luasoap
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=LuaSOAP
  URL:=https://github.com/tomasguisasola/luasoap
  DEPENDS:=+lua +luaexpat +luasec +luasocket
  PKGARCH:=all
endef

define Package/luasoap/description
  LuaSOAP is a library of functions to deal with SOAP.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luasoap/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/soap.lua $(1)/usr/lib/lua/
	$(INSTALL_DIR) $(1)/usr/lib/lua/soap
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/{client,server}.lua $(1)/usr/lib/lua/soap/
	$(INSTALL_DIR) $(1)/usr/lib/lua/soap/client
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/client/https.lua $(1)/usr/lib/lua/soap/client/
	$(INSTALL_DIR) $(1)/usr/lib/lua/soap/tests
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/tests/test{,-http,-server}.lua $(1)/usr/lib/lua/soap/tests/
endef

$(eval $(call BuildPackage,luasoap))
