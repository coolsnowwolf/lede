#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-copas
PKG_VERSION:=2.0.2
PKG_RELEASE:=1

PKG_SOURCE:=copas-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/keplerproject/copas/tar.gz/$(PKG_VERSION)?
PKG_HASH:=1469eaf987513f5a9a0bee345c040c8fa30ca6194bc75ad78171362f9904cf3e
PKG_BUILD_DIR:=$(BUILD_DIR)/copas-$(PKG_VERSION)

PKG_MAINTAINER:=Dirk Chang <dirk@kooiot.com>
PKG_LICENSE:=MIT

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lua-copas
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua-Copas
  URL:=https://github.com/keplerproject/copas
  DEPENDS:=+lua
endef

define Package/lua-copas/description
  Copas is a dispatcher based on coroutines that can be used by TCP/IP servers.
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		T="$(BUILD_VARIANT)" \
		PREFIX="$(PKG_INSTALL_DIR)/usr" \
		install
endef

define Package/lua-copas/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/copas
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/copas.lua $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/copas/ftp.lua $(1)/usr/lib/lua/copas
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/copas/smtp.lua $(1)/usr/lib/lua/copas
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/copas/http.lua $(1)/usr/lib/lua/copas
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/copas/limit.lua $(1)/usr/lib/lua/copas
endef

$(eval $(call BuildPackage,lua-copas))
