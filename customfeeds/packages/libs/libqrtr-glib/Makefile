#
# Copyright (C) 2016 Velocloud Inc.
# Copyright (C) 2016 Aleksander Morgado <aleksander@aleksander.es>
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libqrtr-glib
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.freedesktop.org/software/libqmi/
PKG_HASH:=30d879b2ade6f8f461def3a677755db5c0238babf688d5c83c03b3e6abe35cee

PKG_MAINTAINER:=Nicholas Smith <nicholas.smith@telcoantennas.com.au>

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libqrtr-glib
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2
  TITLE:=Helper library to talk to QRTR enabled modems
  URL:=https://gitlab.freedesktop.org/mobile-broadband/libqrtr-glib
  LICENSE:=LGPL-2.0-or-later
  LICENSE_FILES:=COPYING.LIB
endef

define Package/libqrtr-glib/description
  Helper library talk to QRTR enabled modems.
endef

CONFIGURE_ARGS += \
	--disable-static \
	--disable-gtk-doc \
	--disable-gtk-doc-html \
	--disable-gtk-doc-pdf \
	--disable-silent-rules

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libqrtr-glib \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libqrtr-glib*.so* \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/qrtr-glib.pc \
		$(1)/usr/lib/pkgconfig
endef

define Package/libqrtr-glib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libqrtr-glib*.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libqrtr-glib))
