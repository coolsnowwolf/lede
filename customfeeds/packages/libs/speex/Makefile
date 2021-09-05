#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=speex
PKG_VERSION:=1.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.us.xiph.org/releases/speex/
PKG_HASH:=eaae8af0ac742dc7d542c9439ac72f1f385ce838392dc849cae4536af9210094

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_CPE_ID:=cpe:/a:xiph:speex

include $(INCLUDE_DIR)/package.mk

define Package/libspeex
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Open source speech compression codec library
  URL:=http://www.speex.org/
  MAINTAINER:=Peter Wagner <tripolar@gmx.at>
endef

define Package/libspeex/description
	Open source patent-free speech compression codec library.
	Speex is an Open Source/Free Software patent-free audio compression
	format designed for speech. The Speex Project aims to lower the
	barrier of entry for voice applications by providing a free
	alternative to expensive proprietary speech codecs. Moreover, Speex
	is well-adapted to Internet applications and provides useful features
	that are not present in most other codecs.

	This package contains the shared codec library, needed by other programs.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/speex $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libspeex.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/speex.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libspeex/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libspeex.so.* $(1)/usr/lib/
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-binaries \
	$(if $(CONFIG_SOFT_FLOAT),--enable-fixed-point --disable-float-api --disable-vbr)

$(eval $(call BuildPackage,libspeex))
