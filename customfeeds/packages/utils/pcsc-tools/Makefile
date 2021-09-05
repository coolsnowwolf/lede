#
# Copyright (C) 2017 Daniel Engberg <daniel.engberg.lists@pyret.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pcsc-tools
PKG_VERSION:=1.5.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://ludovic.rousseau.free.fr/softwares/pcsc-tools/
PKG_HASH:=e0ea8f8496d5bcf5316da913869ba95b925d0405c2aaade801c0d6ce7697699d

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/pcsc-tools
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libpcsclite
  TITLE:=pcsc-tools
  URL:=http://ludovic.rousseau.free.fr/softwares/pcsc-tools/
endef

define Package/pcsc-tools/description
  Tools that are used to test a PC/SC driver, card or reader.
  Only includes pcsc_scan without ATR analysis for now.
endef

define Package/pcsc-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pcsc_scan $(1)/usr/bin
endef

$(eval $(call BuildPackage,pcsc-tools))
