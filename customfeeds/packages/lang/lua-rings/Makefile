#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-rings
PKG_VERSION:=1.3.0
PKG_RELEASE:=1
PKG_MAINTAINER:=Dirk Chang <dirk@kooiot.com>
PKG_LICENSE:=MIT

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MIRROR_HASH:=7026ce5898d37a4175b7afad693d59ff1fff1a84adafcabfafd065956a03616c
PKG_SOURCE_URL:=https://github.com/keplerproject/rings.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=2b8a900f8b9dbde304859a3ac9d437795c3fdde3
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_FIXUP:=autoreconf

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lua-rings
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua-Rings
  URL:=http://keplerproject.github.io/rings/
  DEPENDS:=+lua
endef

define Package/lua-rings/description
  Rings is a library which provides a way to create new Lua states from within Lua.
endef

TARGET_CFLAGS += $(FPIC)
# add make variable overrides here
MAKE_FLAGS +=

define Build/Configure
endef

define Package/lua-rings/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/rings.so $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/stable.lua $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lua-rings))
