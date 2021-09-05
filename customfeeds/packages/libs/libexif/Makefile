#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libexif
PKG_VERSION:=0.6.22
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/$(PKG_NAME)/$(PKG_NAME)/releases/download/$(PKG_NAME)-0_6_22-release/
PKG_HASH:=5048f1c8fc509cc636c2f97f4b40c293338b6041a5652082d5ee2cf54b530c56

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:libexif:libexif

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libexif
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=library for jpeg files with exif tags
  URL:=https://libexif.github.io/
  MAINTAINER:=W. Michael Petullo <mike@flyn.org>
endef

define Package/libexif/description
  libexif is a library for parsing, editing, and saving EXIF data. It is
  intended to replace lots of redundant implementations in command-line
  utilities and programs with GUIs.
endef

CONFIGURE_ARGS+= \
	--enable-shared \
	--enable-static \
	--disable-rpath \
	--disable-nls \
	--without-libiconv-prefix \
	--without-libintl-prefix \

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libexif $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libexif*.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libexif.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libexif/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libexif*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libexif))
