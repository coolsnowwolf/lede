#
# Copyright (C) 2023 Jeffery To
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mpfr
PKG_VERSION:=4.2.1
PKG_RELEASE:=1

PKG_SOURCE_URL:=@GNU/mpfr http://www.mpfr.org/mpfr-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2

PKG_LICENSE:=LGPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING.LESSER
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>
PKG_CPE_ID:=cpe:/a:mpfr:gnu_mpfr

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmpfr
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU MPFR library
  URL:=https://www.mpfr.org/
  DEPENDS:=+libgmp
  ABI_VERSION:=6
endef

define Package/libmpfr/description
MPFR is a portable library written in C for arbitrary precision
arithmetic on floating-point numbers. It is based on the GNU MP library.
It aims to provide a class of floating-point numbers with precise
semantics.
endef

CONFIGURE_ARGS += \
	--enable-thread-safe

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/mpf* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmpfr.{a,so*} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mpfr.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libmpfr/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmpfr.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmpfr))
