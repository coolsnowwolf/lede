#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libusb-compat
PKG_VERSION:=0.1.5
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/libusb
PKG_HASH:=404ef4b6b324be79ac1bfb3d839eac860fbc929e6acb1ef88793a6ea328bc55a

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=LGPL-2.1

PKG_MAINTAINER := Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk

define Package/libusb-compat
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libusb-0.1 compatibility library
  DEPENDS:=+libusb-1.0
  URL:=http://libusb.wiki.sourceforge.net/
  ABI_VERSION:=4
endef

define Package/libusb-compat/description
  libusb is a C library that gives applications easy access to USB devices on
  many different operating systems.
endef

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
	$(INSTALL_DIR) $(2)/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/libusb-config $(2)/bin/
endef

define Package/libusb-compat/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusb-0.1.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libusb-compat))
