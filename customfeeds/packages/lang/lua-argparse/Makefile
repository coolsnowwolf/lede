#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-argparse
PKG_VERSION:=0.6.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/mpeterv/argparse/tar.gz/$(PKG_VERSION)?
PKG_HASH:=0eddda29d591536bc7310b99ce7acc3e5e00557f18d6e63ab10d56683e8952f1
PKG_BUILD_DIR:=$(BUILD_DIR)/argparse-$(PKG_VERSION)

PKG_MAINTAINER:=Vladimir Malyutin <first-leon@yandex.ru>
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/lua-argparse
        SUBMENU:=Lua
        SECTION:=lang
        CATEGORY:=Languages
        TITLE:=lua-argparse
        URL:=https://github.com/mpeterv/argparse
        DEPENDS:=+lua
        PKGARCH:=all
endef

define Package/lua-argparse/description
        Argparse is a feature-rich command line parser for Lua inspired by argparse for Python.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/lua-argparse/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/argparse.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-argparse))
