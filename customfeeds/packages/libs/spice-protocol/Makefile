#
# Copyright (C) 2019-2020 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=spice-protocol
PKG_VERSION:=0.14.3
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.spice-space.org/download/releases
PKG_HASH:=f986e5bc2a1598532c4897f889afb0df9257ac21c160c083703ae7c8de99487a
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk


define Package/spice-protocol
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=SPICE - headers defining protocols
  URL:=https://www.spice-space.org/index.html
  BUILDONLY:=1
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/share/pkgconfig/* $(1)/usr/lib/pkgconfig
endef

$(eval $(call BuildPackage,spice-protocol))
