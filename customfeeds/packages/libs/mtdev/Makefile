#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtdev
PKG_VERSION:=1.1.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://bitmath.org/code/mtdev/
PKG_HASH:=15d7b28da8ac71d8bc8c9287c2045fd174267bc740bec10cfda332dc1204e0e0

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mtdev
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Multitouch Protocol Translation Library
  URL:=http://bitmath.org/code/mtdev/
endef

define Package/mtdev/description
  The mtdev is a stand-alone library which transforms all variants of
  kernel MT events to the slotted type B protocol. The events put into
  mtdev may be from any MT device, specifically type A without contact
  tracking, type A with contact tracking, or type B with contact
  tracking. See the kernel documentation for further details.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
endef

define Package/mtdev/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,mtdev))
