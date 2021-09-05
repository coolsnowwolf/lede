#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-mobdebug
PKG_VERSION:=0.70
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/pkulchenko/MobDebug/tar.gz/$(PKG_VERSION)?
PKG_HASH:=35ec131a0ecc74dbe1cc50582ed977120a4ef6d9f815ce07367fdb945cfee370
PKG_BUILD_DIR:=$(BUILD_DIR)/MobDebug-$(PKG_VERSION)

PKG_MAINTAINER:=Dirk Chang <dirk@kooiot.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lua-mobdebug
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua-MobDebug
  URL:=https://github.com/pkulchenko/MobDebug
  DEPENDS:=+lua
  PKGARCH:=all
endef

define Package/lua-mobdebug/description
  MobDebug is a remote debugger for Lua (including Lua 5.1, Lua 5.2, Lua 5.3, and LuaJIT 2.x).
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/Install
endef

# add make variable overrides here
MAKE_FLAGS +=

define Package/lua-mobdebug/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/mobdebug.lua $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lua-mobdebug))
