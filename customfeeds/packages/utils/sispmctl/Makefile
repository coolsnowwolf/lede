#
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sispmctl
PKG_VERSION:=4.8
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=0f8391f7e95cbf1fb96a68686a1dcf1e16747b050ae1b8ff90653c99976068db

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libsispmctl
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Silver Shield PM Control for Linux library
  URL:=http://sispmctl.sourceforge.net
  DEPENDS:=+libusb-compat
endef

define Package/sispmctl
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Silver Shield PM Control for Linux
  URL:=http://sispmctl.sourceforge.net
  DEPENDS:=+libsispmctl
endef

define Package/sispmctl/description
 The sispmctl tool can control Gembird SIS-PM Silver Shield
 programmable power outlet strips (also known under the name
 Revolt Intelli-Plug) from the command line.
 .
 It can be used to switch on or off any of the programmable
 power sockets of the SIS-PM via USB. It can also show the
 current status of each power socket, and it can handle
 multiple SIS-PM devices, too.
endef

CONFIGURE_ARGS += \
	--enable-webless \
	--disable-dependency-tracking

define Package/libsispmctl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsispmctl.so* $(1)/usr/lib/
endef

define Package/sispmctl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sispmctl $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libsispmctl))
$(eval $(call BuildPackage,sispmctl))
