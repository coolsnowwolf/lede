#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=flac
PKG_VERSION:=1.3.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://downloads.xiph.org/releases/flac/
PKG_HASH:=213e82bd716c9de6db2f98bcadbc4c24c7e2efe8c75939a1a84e28539c4e1748

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GFDL-1.2 GPL-2 LGPL-2.1 BSD-3-Clause
PKG_LICENSE_FILES:=README COPYING.FDL COPYING.GPL COPYING.LGPL COPYING.Xiph
PKG_CPE_ID:=cpe:/a:flac_project:flac

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_REMOVE_FILES:=autogen.sh aclocal.m4

include $(INCLUDE_DIR)/package.mk

define Package/libflac
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Free Lossless Audio Codec library
  URL:=https://xiph.org/flac
endef

CONFIGURE_ARGS += \
	--disable-cpplibs \
	--disable-sse \
	--disable-altivec \
	--disable-doxgen-docs \
	--disable-local-xmms-plugin \
	--disable-xmms-plugin \
	--disable-ogg \
	--disable-oggtest \
	--disable-thorough-tests \
	--disable-examples \
	--disable-rpath \
	$(if $(CONFIG_DEBUG),--enable-debug) \
	--enable-static \
	--without-pic

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/FLAC \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libFLAC.{a,so*,la} \
		$(1)/usr/lib/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* \
		$(1)/usr/lib/pkgconfig/

	$(INSTALL_DIR) $(1)/usr/share/aclocal/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/aclocal/* \
		$(1)/usr/share/aclocal/
endef

define Package/libflac/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libFLAC.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libflac))
