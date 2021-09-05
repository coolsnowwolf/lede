#
# Copyright (C) 2009-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Comment: This makefile is based on the oldpackage makefile by acinonyx / nbd
# https://dev.openwrt.org/browser/packages/utils/digitemp/Makefile

include $(TOPDIR)/rules.mk

PKG_NAME:=digitemp
PKG_VERSION:=3.7.2
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/bcl/digitemp/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=683df4ab5cc53a45fe4f860c698f148d34bcca91b3e0568a342f32d64d12ba24

PKG_MAINTAINER:=Jasper Scholte <NightNL@outlook.com>
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/digitemp/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=DigiTemp is a simple to use program for reading values from 1-wire devices.
  URL:=https://www.digitemp.com
endef

define Package/digitemp/Default/description
	DigiTemp is a simple to use program for reading values from 1-wire devices.
	Its main use is for reading temperature sensors, but it also reads counters,
	and understands the 1-wire hubs with devices on different branches of the network.
endef

define Package/digitemp
$(call Package/digitemp/Default)
  TITLE += (serial version)
endef

define Package/digitemp/description
$(call Package/digitemp/Default/description)
	This package is built without USB support.
endef

define Package/digitemp-usb
$(call Package/digitemp/Default)
  TITLE += (USB version)
  DEPENDS += +libusb-compat
endef

define Package/digitemp-usb/description
$(call Package/digitemp/Default/description)
	This package is built with USB support.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -DLINUX $(TARGET_CPPFLAGS) \
			-I./src -I./userial" \
		LIBS="$(TARGET_LDFLAGS)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		ds9097 ds9097u
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -DLINUX $(TARGET_CPPFLAGS) \
			-I./src -I./userial -DOWUSB" \
		LIBS="$(TARGET_LDFLAGS) -lusb -lm -DOWUSB" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		clean ds2490
endef

define Package/digitemp/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/digitemp_{DS9097,DS9097U} $(1)/usr/bin/
endef

define Package/digitemp-usb/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/digitemp_DS2490 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,digitemp))
$(eval $(call BuildPackage,digitemp-usb))
