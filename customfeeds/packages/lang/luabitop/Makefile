#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luabitop
PKG_VERSION:=1.0.2
PKG_RELEASE:=1

_BASENAME:=LuaBitOp

PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_SOURCE:=$(_BASENAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://bitop.luajit.org/download/
PKG_HASH:=1207c9293dcd52eb9dca6538d1b87352bd510f4e760938f5048433f7f272ce99
PKG_BUILD_DIR:=$(BUILD_DIR)/$(_BASENAME)-$(PKG_VERSION)
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/luabitop
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=luabitop
  URL:=http://bitop.luajit.org/
  DEPENDS:=+liblua
endef

define Package/luabitop/description
Lua BitOp is a C extension module for Lua 5.1/5.2 which adds bitwise operations on numbers.
endef

define Build/Configure
endef


TARGET_CFLAGS += $(FPIC) -DLUA_USE_LINUX -DLUA_NUMBER_DOUBLE

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(TARGET_CPPFLAGS) -std=gnu99 $(FPIC) -DLUA_USE_LINUX -shared -o $(PKG_BUILD_DIR)/bit.so $(PKG_BUILD_DIR)/bit.c
endef

define Package/luabitop/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bit.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,luabitop))
