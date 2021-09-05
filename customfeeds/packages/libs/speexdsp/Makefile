#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=speexdsp
PKG_VERSION:=1.2.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://downloads.us.xiph.org/releases/speex/
PKG_HASH:=682042fc6f9bee6294ec453f470dadc26c6ff29b9c9e9ad2ffc1f4312fd64771

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libspeexdsp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Open source speech compression codec library output to DSP
  URL:=http://www.speex.org/
  MAINTAINER:=Peter Wagner <tripolar@gmx.at>
endef

define Package/libspeexdsp/description
	Open source patent-free speech compression codec library.
	Speex is an Open Source/Free Software patent-free audio compression
	format designed for speech. The Speex Project aims to lower the
	barrier of entry for voice applications by providing a free
	alternative to expensive proprietary speech codecs. Moreover, Speex
	is well-adapted to Internet applications and provides useful features
	that are not present in most other codecs.

	This package contains the shared dsp library, needed by other programs.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/speex $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libspeexdsp.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/speexdsp.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libspeexdsp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libspeexdsp.so.* $(1)/usr/lib/
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-examples \
	$(if $(CONFIG_SOFT_FLOAT),--enable-fixed-point --disable-float-api)

$(eval $(call BuildPackage,libspeexdsp))
