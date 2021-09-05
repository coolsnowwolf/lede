#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmpeg2
PKG_VERSION:=0.5.1
PKG_RELEASE:=1

PKG_SOURCE:=libmpeg2-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://libmpeg2.sourceforge.net/files/
PKG_HASH:=dee22e893cb5fc2b2b6ebd60b88478ab8556cb3b93f9a0d7ce8f3b61851871d4

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmpeg2
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=MPEG-1 & -2 decoding library
  URL:=http://libmpeg2.sourceforge.net/
  MAINTAINER:=W. Michael Petullo <mike@flyn.org>
endef

define Package/libmpeg2/decription
  Libmpeg2 is a library for decoding MPEG-1 and MPEG-2 video streams
endef

CONFIGURE_ARGS += \
	--disable-sdl \
	--without-x \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mpeg2dec \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmpeg2{,convert}.{a,so*} \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libmpeg2{,convert}.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libmpeg2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmpeg2{,convert}.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmpeg2))
