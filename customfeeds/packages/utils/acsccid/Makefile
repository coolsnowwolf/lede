#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=acsccid
PKG_VERSION:=1.1.8
PKG_RELEASE:=1

PKG_SOURCE:=v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/acshk/acsccid/archive/
PKG_HASH:=68d15eb20e7f52153509f1dc300cf0c68b388c59d7d124ba494fd96c61a6e7c6
PKG_MAINTAINER:=Vincent JARDIN <vjardin@free.fr>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/acsccid
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=$(ICONV_DEPENDS) +libusb-1.0 +libpcsclite
  TITLE:=PCSC driver for ACS USB CCID smart card readers
  URL:=https://github.com/acshk/acsccid
endef

define Package/acsccid/description
  PCSC driver for ACS USB CCID (Chip/Smart Card Interface Devices)
  smart card readers and ICCD (Integrated Circuit(s)
  Card Devices).
endef

TARGET_CFLAGS += $(FPIC)

TARGET_LDFLAGS += "-lpthread"

CONFIGURE_ARGS += \
	--enable-embedded \
	--enable-usbdropdir=/usr/lib/pcsc/drivers

define Package/acsccid/install
	$(INSTALL_DIR) $(1)/usr/lib/pcsc
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pcsc/drivers $(1)/usr/lib/pcsc/
endef

$(eval $(call BuildPackage,acsccid))
