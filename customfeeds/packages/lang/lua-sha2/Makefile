#
# Copyright (C) 20013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-sha2
PKG_VERSION:=0.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MIRROR_HASH:=79e65bd7cb1b8ef64e125c9e57f40b7d38678599587c1894ce57838814403945
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/lgierth/lua-sha2.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=02bd4bfdc8062c7e1ae51c5a72906efc0a6375ee
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/lua-sha2
	SUBMENU:=Lua
	SECTION:=lang
	CATEGORY:=Languages
	TITLE:=Lua binding for Aaron Gifford's SHA-2 implementation
	URL:=https://code.google.com/p/sha2/
	MAINTAINER:=Lars Gierth <larsg@systemli.org>
	DEPENDS:=+lua
endef

define Package/lua-sha2/description
	Lua Binding for the SHA-2 (SHA-256/384/512) BSD-licensed C implementation by Aaron Gifford.
	Also contains a HMAC implementation in Lua.
endef

TARGET_CFLAGS += -fPIC -DSHA2_USE_INTTYPES_H -DBYTE_ORDER -DLITTLE_ENDIAN
TARGET_LDFLAGS += -fPIC -DSHA2_USE_INTTYPES_H -DBYTE_ORDER -DLITTLE_ENDIAN

define Build/Configure
endef

define Package/lua-sha2/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/hmac
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/sha2.so $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/hmac.lua $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/hmac/sha2.lua $(1)/usr/lib/lua/hmac
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/hmac/md5.lua $(1)/usr/lib/lua/hmac
endef

$(eval $(call BuildPackage,lua-sha2))
