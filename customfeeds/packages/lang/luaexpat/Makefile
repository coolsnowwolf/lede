# 
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luaexpat
PKG_VERSION:=1.3.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://matthewwild.co.uk/projects/luaexpat
PKG_HASH:=d060397960d87b2c89cf490f330508b7def1a0677bdc120531c571609fc57dc3

PKG_CPE_ID:=cpe:/a:matthewwild:luaexpat

include $(INCLUDE_DIR)/package.mk

define Package/luaexpat
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=LuaExpat 
  URL:=http://matthewwild.co.uk/projects/luaexpat/
  MAINTAINER:=W. Michael Petullo <mike@flyn.org>
  DEPENDS:=+lua +libexpat
endef

define Package/luaexpat/description
  LuaExpat is a SAX XML parser based on the Expat library.
endef

define Build/Configure
endef

define Build/Compile
	$(CP) files/compat-5.1r5 $(PKG_BUILD_DIR)/compat-5.1r5
	$(MAKE) -C $(PKG_BUILD_DIR) \
	EXPAT_INC="-I$(STAGING_DIR)/usr/include/" \
	LUA_INC="-I$(STAGING_DIR)/usr/include/" \
	LUA_LIBDIR="$(STAGING_DIR)/usr/lib/" \
	COMPAT_DIR="$(PKG_BUILD_DIR)/compat-5.1r5" \
	LDFLAGS="-shared $(TARGET_LDFLAGS)" \
	CC="$(TARGET_CC) $(TARGET_CFLAGS) $(FPIC) -std=c99" \
	LD="$(TARGET_CROSS)ld -shared" 
endef

define Package/luaexpat/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/lxp.so $(1)/usr/lib/lua/lxp.so
	$(INSTALL_DIR) $(1)/usr/lib/lua/lxp
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/lxp/lom.lua $(1)/usr/lib/lua/lxp
endef

$(eval $(call BuildPackage,luaexpat))
