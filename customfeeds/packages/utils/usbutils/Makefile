#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=usbutils
PKG_VERSION:=013
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/usb/usbutils
PKG_HASH:=9e23494fcc78b7a80ee29a07dd179c95ae2f71509c35728dbbabc2d1cca41338

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/usbutils
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libusb-1.0 +libudev +usbids +librt +libpthread
  TITLE:=USB devices listing utilities
  URL:=http://www.linux-usb.org/
endef

CONFIGURE_ARGS += \
	--datadir=$(CONFIGURE_PREFIX)/share/hwdata

define Package/usbutils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lsusb $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usbreset $(1)/usr/bin/
endef

$(eval $(call BuildPackage,usbutils))
