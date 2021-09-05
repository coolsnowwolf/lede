#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libusb-compat
PKG_VERSION:=0.1.7
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/libusb/libusb-compat-0.1/releases/download/v$(PKG_VERSION)
PKG_HASH:=8259f8d5b084fe43c47823a939e955e0ba21942b8d112266c39d228cc14764d6

PKG_MAINTAINER:= Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

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
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/libusb-config $(1)/usr/bin/
	$(SED) \
		's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' \
		$(1)/usr/bin/libusb-config
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/libusb-config $(2)/bin/
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/usb.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusb* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libusb.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libusb-compat/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusb-0.1.so.$(ABI_VERSION)* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libusb-compat))
