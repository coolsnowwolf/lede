#
# Copyright (C) 2017 Bruno Randolf (br1@einfach.org)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dfu-util
PKG_VERSION:=0.10
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_MAINTAINER:=Bruno Randolf <br1@einfach.org>
PKG_HASH:=a03dc58dfc79c056819c0544b2a5970537566460102b3d82cfb038c60e619b42

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/dfu-util
  SUBMENU:=
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libusb-1.0
  TITLE:=Device Firmware Upgrade Utilities
  URL:=http://dfu-util.sourceforge.net/
  MAINTAINER:=Bruno Randolf <br1@einfach.org>
endef

define Package/dfu-util/description
	dfu-util is a host side implementation of the DFU 1.0 and DFU 1.1
	specifications of the USB forum. DFU is intended to download and
	upload firmware to/from devices connected over USB.
endef

define Package/dfu-util/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/dfu-util $(1)/usr/bin/
endef

$(eval $(call BuildPackage,dfu-util))
