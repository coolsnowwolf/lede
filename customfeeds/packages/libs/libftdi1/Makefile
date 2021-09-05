#
# Copyright (C) 2014-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libftdi1
PKG_VERSION:=1.5
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.intra2net.com/en/developer/libftdi/download/
PKG_HASH:=7c7091e9c86196148bd41177b4590dccb1510bfe6cea5bf7407ff194482eb049

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>
PKG_LICENSE:=LGPL-2.1-only
PKG_LICENSE_FILES:=COPYING.LIB

CMAKE_INSTALL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libftdi1
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libusb-1.0
  TITLE:=Library to talk to FTDI chips
  URL:=http://www.intra2net.com/en/developer/libftdi/
endef

define Package/libftdi1/description
  libFTDI - FTDI USB driver with bitbang mode
  libFTDI is an open source library to talk to FTDI chips: FT232BM, FT245BM, FT2232C, FT2232H, FT4232H, FT2232D and FT245R, including the popular bitbang mode. 
  The library is linked with your program in userspace, no kernel driver required.
endef

define Package/ftdi_eeprom
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+confuse +libftdi1
  TITLE:=Tool for reading/erasing/flashing FTDI USB chip eeproms
  URL:=http://www.intra2net.com/en/developer/libftdi/
endef

define Package/ftdi_eeprom/description
  ftdi_eeprom is a small tool for reading/erasing/flashing FTDI USB chip
  eeproms. It uses libftdi to access the chip, so you will need to have
  the required permissions on the device.

  The ftdi_sio module should not be loaded.

  You have to unplug and replug your device to get the new values to be
  read. Otherwise, you will still get the old values.
endef

CMAKE_OPTIONS += \
	-DBUILD_TESTS=OFF \
	-DDOCUMENTATION=OFF \
	-DEXAMPLES=OFF

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,/usr/bin,/usr,g' $(1)/usr/lib/pkgconfig/libftdi1.pc
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libftdi1.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libftdi1.pc
	$(SED) 's,/usr/bin,/usr,g' $(1)/usr/lib/pkgconfig/libftdipp1.pc
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libftdipp1.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libftdipp1.pc
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/libftdi1-config
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/libftdi1-config $(2)/bin/libftdi1-config
endef

define Package/libftdi1/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/libftdi1-config $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libftdi1.so.* $(1)/usr/lib/
endef

define Package/ftdi_eeprom/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ftdi_eeprom $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libftdi1))
$(eval $(call BuildPackage,ftdi_eeprom))
