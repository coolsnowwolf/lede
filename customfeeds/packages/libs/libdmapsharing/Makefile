#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# This Makefile is a skeleton
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdmapsharing
PKG_VERSION:=3.9.10
PKG_RELEASE:=1

PKG_SOURCE:=libdmapsharing-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.flyn.org/projects/libdmapsharing/
PKG_HASH:=e216425d61a109f08f0ac93065f4603d1e61ebd85c09802c03a9ecce5bb35094

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=glib2/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libdmapsharing
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libsoup +mdnsresponder +gstreamer1-plugins-base +gst1-mod-app
  TITLE:=libdmapsharing
  URL:=https://www.flyn.org/projects/libdmapsharing/
endef

define Package/libdmapsharing/decription
  Libdmapsharing is a DMAP library implementation in C
endef

CONFIGURE_ARGS += \
	--disable-tests \
	--disable-gtk-doc \
	--disable-introspection

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libdmapsharing-4.0/ \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libdmapsharing/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdmapsharing))
