#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mktorrent
PKG_VERSION:=1.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/Rudde/$(PKG_NAME)
PKG_SOURCE_VERSION:=b20ef699b4ee5ded2f078ead776c7deac969e19a
PKG_MIRROR_HASH:=abe5f96d1a91c00bd6dcf86640a205583e8607d2b0e9299a588cec76c556a8e8
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/mktorrent
  SUBMENU:=BitTorrent
  SECTION:=net
  CATEGORY:=Network
  TITLE:=mktorrent
  URL:=https://github.com/Rudde/mktorrent
endef

define Package/mktorrent/Description
 mktorrent is a simple command line utility to create BitTorrent metainfo files.
endef


define Package/mktorrent/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mktorrent $(1)/usr/bin
endef

$(eval $(call BuildPackage,mktorrent))
