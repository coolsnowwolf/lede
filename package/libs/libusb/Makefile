#
# Copyright (C) 2010-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libusb
PKG_VERSION:=1.0.24
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=\
  https://github.com/libusb/libusb/releases/download/v$(PKG_VERSION) \
  @SF/$(PKG_NAME)
PKG_HASH:=7efd2685f7b327326dcfb85cee426d9b871fd70e22caa15bb68d595ce2a2b12a

PKG_MAINTAINER:= Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libusb-1.0
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A library for accessing Linux USB devices
  DEPENDS:=+libpthread +librt
  URL:=http://libusb.info/
  ABI_VERSION:=0
endef

define Package/libusb-1.0/description
  libusb is a C library that gives applications easy access to USB devices on
  many different operating systems.
endef

define Package/fxload
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=fxload firmware loader
  URL:=http://linux-hotplug.sourceforge.net
  DEPENDS:=+libusb-1.0
endef

define Package/fxload/description
This program is conveniently able to download firmware into FX, FX2,
and FX2LP EZ-USB devices, as well as the original AnchorChips EZ-USB.
It is intended to be invoked by hotplug scripts when the unprogrammed
device appears on the bus.
endef

TARGET_CFLAGS += $(FPIC)
CONFIGURE_ARGS += \
	--enable-examples-build \
	--disable-log \
	--disable-udev

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libusb-1.0
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libusb-1.0/libusb.h $(1)/usr/include/libusb-1.0/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusb-1.0.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libusb-1.0.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libusb-1.0/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusb-1.0.so.* $(1)/usr/lib/
endef

define Package/fxload/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/examples/.libs/fxload $(1)/usr/sbin
endef

$(eval $(call BuildPackage,libusb-1.0))
$(eval $(call BuildPackage,fxload))
