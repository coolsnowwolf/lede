#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=spawn-fcgi
PKG_VERSION:=1.6.4
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://download.lighttpd.net/spawn-fcgi/releases-1.6.x/
PKG_HASH:=423b0c317e0084773b483985cc21930c4b8dfcb411f7353d6ee6fc41d9cb9d45

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/spawn-fcgi
  SUBMENU:=Web Servers/Proxies
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Tool to spawn fastcgi applications
  URL:=http://redmine.lighttpd.net/projects/spawn-fcgi
endef

define Package/spawn-fcgi/description
	spawn-fcgi is used to spawn fastcgi applications
endef

define Package/spawn-fcgi/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/spawn-fcgi $(1)/usr/bin/
endef

$(eval $(call BuildPackage,spawn-fcgi))
