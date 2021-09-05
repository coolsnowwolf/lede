#
# Copyright (C) 20013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dkjson
PKG_VERSION:=2.5
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/LuaDist/dkjson/tar.gz/$(PKG_VERSION)?
PKG_HASH:=8560ee3c7ed421b293bf0905017bc90bd2f282d56517aa59ec6c53f5fb4057bd

PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/dkjson
	SUBMENU:=Lua
	SECTION:=lang
	CATEGORY:=Languages
	TITLE:=DKJSON
	URL:=http://dkolf.de/src/dkjson-lua.fsl/home
	MAINTAINER:=Lars Gierth <larsg@systemli.org>
	DEPENDS:=+lua
	PKGARCH:=all
endef

define Package/dkjson/description
	Lua JSON parser/serializer with UTF-8 support
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/dkjson/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dkjson.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,dkjson))
