# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=relayctl
PKG_VERSION:=0.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/relayctl/relayctl-$(PKG_VERSION)
PKG_MAINTAINER:=Heinrich Schuchardt <xypron.glpk@gmx.de>
PKG_HASH:=74ce41ebd5d18816d52b5f9b1071f658f5a814b8194257f20700214bbee9de1e
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/relayctl
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Simple command-line control of SainSmart USB relay boards
  URL:=https://github.com/darryln/relayctl
  DEPENDS:= +libftdi1
endef

define Package/relayctl/description
 Simple command-line control of SainSmart USB relay boards based on
 FTDI chips using bitbang mode.
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libftdi1

define Package/relayctl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/lib/udev/rules.d/
	$(CP) $(PKG_BUILD_DIR)/examples/60-relayctl.rules $(1)/lib/udev/rules.d/
endef

$(eval $(call BuildPackage,relayctl))
