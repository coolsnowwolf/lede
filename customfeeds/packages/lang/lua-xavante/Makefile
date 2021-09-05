#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-xavante
PKG_VERSION:=2.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/keplerproject/xavante/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=0e4f49dd96cb092cd5d80cc66a5204dcb22a14f80897a121f2f0d1dceb1c7ba5
PKG_BUILD_DIR:=$(BUILD_DIR)/xavante-$(PKG_VERSION)

PKG_MAINTAINER:=Dirk Chang <dirk@kooiot.com>
PKG_LICENSE:=MIT

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lua-xavante
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Xavante Web Server
  URL:=https://keplerproject.github.io/xavante/
  DEPENDS:= +lua
  PKGARCH:=all
endef

define Package/lua-xavante/description
  Xavante is a Lua HTTP 1.1 Web server that uses a modular architecture based on URI mapped handlers.
endef


define Build/Configure
endef

define Build/Compile
endef

define Build/Install
endef

define Package/lua-xavante/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/sajax/sajax.lua $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/xavante/xavante.lua $(1)/usr/lib/lua

	$(INSTALL_DIR) $(1)/usr/lib/lua/xavante
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/xavante/{cgiluahandler,encoding,filehandler,httpd,mime,patternhandler,redirecthandler,vhostshandler,indexhandler,urlhandler,ruleshandler}.lua $(1)/usr/lib/lua/xavante
endef

$(eval $(call BuildPackage,lua-xavante))
