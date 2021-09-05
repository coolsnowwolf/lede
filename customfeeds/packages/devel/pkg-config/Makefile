#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pkg-config
PKG_VERSION:=0.29.2
PKG_RELEASE:=3

PKG_SOURCE_URL:=https://pkg-config.freedesktop.org/releases/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=6fc69c01688c9458a57eb9a1664c9aba372ccda420a02bf4429fe610e7e7d591

PKG_MAINTAINER:=Heinrich Schuchardt <xypron.glpk@gmx.de>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/pkg-config
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=pkg-config
  URL:=https://www.freedesktop.org/wiki/Software/pkg-config/
  DEPENDS:=+glib2 $(INTL_DEPENDS)
  CONFLICTS:=pkgconf
endef

define Package/pkg-config/description
  pkg-config is a helper tool used when compiling applications and libraries.
  It helps you insert the correct compiler options on the command line so an
  application can use gcc -o test test.cpkg-config --libs --cflags glib-2.0
  for instance, rather than hard-coding values on where to find glib (or
  other libraries).
endef

TARGET_CFLAGS += $(FPIC)

define Package/pkg-config/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pkg-config $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/aclocal/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/aclocal/pkg.m4 \
	  $(1)/usr/share/aclocal/
endef

$(eval $(call BuildPackage,pkg-config))
