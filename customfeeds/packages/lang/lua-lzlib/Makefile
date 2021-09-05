#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-lzlib
PKG_VERSION:=0.4.3
PKG_RELEASE:=1
PKG_MAINTAINER:=Dirk Chang <dirk@kooiot.com>
PKG_LICENSE:=MIT

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MIRROR_HASH:=b6ef5e3f04b7f2137b39931a175ee802489a2486e70537770919bcccca10e723
PKG_SOURCE_URL:=https://github.com/LuaDist/lzlib.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=79329a07d8f79c19eadd7ea2752b4c4e1574b015
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/lua-lzlib
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua zlib binding
  URL:=http://github.com/LuaDist/lzlib
  DEPENDS:= +lua +zlib
endef

define Package/lua-lzlib/description
	A library to access zlib library functions and also to read/write gzip files using an interface similar to the base io package. 
endef

MAKE_FLAGS += \
	LUA="$(STAGING_DIR)/usr" \
	OFLAGS="$(TARGET_CFLAGS)" \

define Package/lua-lzlib/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zlib.so $(1)/usr/lib/lua/

	$(INSTALL_DATA) $(PKG_BUILD_DIR)/gzip.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-lzlib))
