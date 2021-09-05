#
# Copyright (C) 2012-2015 OpenWrt.org
# Copyright (C) 2017 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=reaver
PKG_VERSION:=1.6.6
PKG_RELEASE:=1
PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/t6x/reaver-wps-fork-t6x/releases/download/v$(PKG_VERSION)
PKG_HASH:=e329a0da0b6dd888916046535ff86a6aa144644561937954e560bb1810ab6702

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=docs/LICENSE

PKG_USE_MIPS16:=0
PKG_AUTOMAKE_PATHS:=src
PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
CONFIGURE_PATH:=src
MAKE_PATH:=src

CONFIGURE_ARGS += --enable-savetocurrent

define Package/reaver
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Wireless
  TITLE:=Efficient brute force attack against Wifi Protected Setup
  URL:=https://github.com/t6x/reaver-wps-fork-t6x
  DEPENDS:=+libpcap
endef

define Package/reaver/description
  Reaver has been designed to be a robust and practical attack against Wi-Fi
  Protected Setup (WPS) registrar PINs in order to recover WPA/WPA2
  passphrases. It has been tested against a wide variety of access points and
  WPS implementations.

  This is reaver-wps-fork-t6x, a community forked version, which has included
  various bug fixes and additional attack method (the offline Pixie Dust
  attack).
endef

define Package/reaver/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/{reaver,wash} $(1)/usr/bin/
endef

$(eval $(call BuildPackage,reaver))
