#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=remserial
PKG_VERSION:=1.4
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0-or-later

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://lpccomp.bc.ca/remserial/
PKG_HASH:=4cbe9c754a2be4926789500bafb84bda54e11b01aab522a3ce71ec4db615fefb

PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/remserial
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Bridge TCP/IP port with a device
  URL:=http://lpccomp.bc.ca/remserial/
endef

define Package/remserial/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/remserial $(1)/usr/bin/
endef

$(eval $(call BuildPackage,remserial))
