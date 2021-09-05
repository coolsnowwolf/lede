#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libupnpp
PKG_VERSION:=0.21.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.lesbonscomptes.com/upmpdcli/downloads
PKG_HASH:=e43dbe5020ed25152aaec71b325dd80498e72038da5aeb50ed7dd51e213cd903

PKG_MAINTAINER:=
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libupnpp
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://www.lesbonscomptes.com/upmpdcli
  DEPENDS+=+libnpupnp
  TITLE:=The libupnpp C++ library wraps libupnp for easier use by upmpdcli and upplay
endef

define Package/libupnpp/description
libupnpp defines useful objects over libupnp and can be used to create both devices
and control points. It is shared by upmpdcli and upplay.
endef

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lm)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libupnpp $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libupnpp.{la,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libupnpp.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libupnpp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libupnpp.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libupnpp))
