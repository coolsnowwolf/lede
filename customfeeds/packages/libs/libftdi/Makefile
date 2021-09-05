#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libftdi
PKG_VERSION:=0.20
PKG_RELEASE:=7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.intra2net.com/en/developer/libftdi/download/
PKG_HASH:=3176d5b5986438f33f5208e690a8bfe90941be501cc0a72118ce3d338d4b838e

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>
PKG_LICENSE:=LGPL-2.0
PKG_LICENSE_FILES:=COPYING.LIB

CMAKE_INSTALL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libftdi
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libusb-compat
  TITLE:=Library to talk to FTDI chips
  URL:=http://www.intra2net.com/en/developer/libftdi/
endef

define Package/libftdi/description
  libFTDI - FTDI USB driver with bitbang mode
  libFTDI is an open source library to talk to FTDI chips: FT232BM, FT245BM, FT2232C, FT2232H, FT4232H, FT2232D and FT245R, including the popular bitbang mode. 
  The library is linked with your program in userspace, no kernel driver required.
endef

CMAKE_OPTIONS += \
	-DBoost_NO_BOOST_CMAKE=ON \
	-DEXAMPLES=OFF

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,/usr/bin,/usr,g' $(1)/usr/lib/pkgconfig/libftdi.pc
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libftdi.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libftdi.pc
	$(SED) 's,/usr/bin,/usr,g' $(1)/usr/lib/pkgconfig/libftdipp.pc
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libftdipp.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libftdipp.pc
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/libftdi-config
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/libftdi-config $(2)/bin/libftdi-config
endef

define Package/libftdi/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/libftdi-config $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libftdi.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libftdi))
