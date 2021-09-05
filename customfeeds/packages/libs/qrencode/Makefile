#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qrencode
PKG_VERSION:=4.1.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://fukuchi.org/works/qrencode
PKG_HASH:=e455d9732f8041cf5b9c388e345a641fd15707860f928e94507b1961256a6923

PKG_MAINTAINER:=Jonathan Bennett <JBennett@incomsystems.biz>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_qrencode

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libqrencode
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for encoding data in a QR Code symbol
  URL:=https://fukuchi.org/works/qrencode/
endef

define Package/libqrencode/description
Libqrencode is a C library for encoding data in a QR Code symbol,
a kind of 2D symbology that can be scanned by handy terminals such
as a mobile phone with CCD. The capacity of QR Code is up to 7000
digits or 4000 characters, and is highly robust.
endef

define Package/qrencode
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=qrencode binary for producing qr codes
  URL:=http://fukuchi.org/works/qrencode/
  DEPENDS:=+libqrencode
endef

define Package/qrencode/description
Qrencode is a C program for encoding data in a QR Code symbol,
a kind of 2D symbology that can be scanned by handy terminals such
as a mobile phone with CCD. The capacity of QR Code is up to 7000
digits or 4000 characters, and is highly robust.
endef

CMAKE_OPTIONS += \
	-DCMAKE_DISABLE_FIND_PACKAGE_PNG=ON \
	-DWITHOUT_PNG=ON \
	-DWITH_TOOLS=O$(if $(CONFIG_PACKAGE_qrencode),N,FF) \
	-DWITH_TESTS=OFF \
	-DBUILD_SHARED_LIBS=ON

TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -Wl,--gc-sections

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/qrencode.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libqrencode.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libqrencode.pc $(1)/usr/lib/pkgconfig/
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libqrencode.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libqrencode.pc
endef

define Package/libqrencode/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libqrencode.so* $(1)/usr/lib/
endef

define Package/qrencode/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/qrencode $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libqrencode))
$(eval $(call BuildPackage,qrencode))
