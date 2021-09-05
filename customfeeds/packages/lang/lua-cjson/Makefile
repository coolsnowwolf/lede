#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-cjson
PKG_VERSION:=2.1.0
PKG_RELEASE:=2
PKG_MAINTAINER:=Dirk Chang <dirk@kooiot.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.kyne.com.au/~mark/software/download/
PKG_HASH:=51bc69cd55931e0cba2ceae39e9efa2483f4292da3a88a1ed470eda829f6c778

HOST_BUILD_DEPENDS:=lua/host

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/lua-cjson
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua CJSON parser
  URL:=https://github.com/mpx/lua-cjson
  DEPENDS:= +lua
endef

define Package/lua-cjson/description
  Lua CJSON is a fast JSON encoding/parsing module for Lua.
endef

CMAKE_OPTIONS += \
	-DUSE_LUA=ON

CMAKE_HOST_OPTIONS += \
	-DLUA_MATH_LIBRARY=m

define Package/lua-cjson/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cjson.so $(1)/usr/lib/lua/

	$(INSTALL_DIR) $(1)/usr/lib/lua/cjson
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lua/cjson/util.lua $(1)/usr/lib/lua/cjson
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,lua-cjson))
