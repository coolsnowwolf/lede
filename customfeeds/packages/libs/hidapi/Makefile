#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hidapi
PKG_VERSION:=0.9.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/libusb/hidapi/tar.gz/$(PKG_NAME)-$(PKG_VERSION)?
PKG_HASH:=630ee1834bdd5c5761ab079fd04f463a89585df8fcae51a7bfe4229b1e02a652
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Paul Fertser <fercerpav@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE-bsd.txt

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/hidapi
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libusb-1.0 +librt +libudev $(ICONV_DEPENDS)
  TITLE:=Library to talk to HID devices
  URL:=https://github.com/libusb/hidapi
endef

define Package/hidapi/description
HIDAPI is a multi-platform library which allows an application to interface
with USB and Bluetooth HID-Class devices on Windows, Linux, FreeBSD, and Mac
OS X.  HIDAPI can be either built as a shared library (.so or .dll) or
can be embedded directly into a target application by adding a single source
file (per platform) and a single header.
endef

CONFIGURE_ARGS += \
	--disable-static

TARGET_LDFLAGS += -liconv

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/hidapi
	$(CP) $(PKG_INSTALL_DIR)/usr/include/hidapi/hidapi.h $(1)/usr/include/hidapi/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhidapi-hidraw.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib//libhidapi-libusb.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/hidapi-libusb.pc $(1)/usr/lib/pkgconfig/
endef

define Package/hidapi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhidapi-hidraw.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhidapi-libusb.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,hidapi))
