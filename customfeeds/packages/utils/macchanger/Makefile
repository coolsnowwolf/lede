#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=macchanger
PKG_VERSION:=1.7.0
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/alobbs/macchanger/releases/download/$(PKG_VERSION)/
PKG_HASH:=dae2717c270fd5f62d790dbf80c19793c651b1b26b62c101b82d5fdf25a845bf

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Nicolas Thill <nico@openwrt.org>

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/macchanger
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=utility for viewing/manipulating the MAC address
  URL:=https://github.com/alobbs/macchanger/
endef

define Package/macchanger/description
	This is a GNU/Linux utility for viewing/manipulating the MAC address
	of network interfaces.
endef

define Package/macchanger/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/macchanger $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/macchanger $(1)/usr/share/
endef

$(eval $(call BuildPackage,macchanger))
