#
# Copyright (C) 2012-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libimobiledevice
PKG_VERSION:=1.3.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.libimobiledevice.org/downloads
PKG_HASH:=53f2640c6365cd9f302a6248f531822dc94a6cced3f17128d4479a77bd75b0f6

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING.LESSER

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libimobiledevice/Default
  TITLE:=A library that talks to Apple devices.
  URL:=https://www.libimobiledevice.org/
  SUBMENU:=libimobiledevice
endef

define Package/libimobiledevice/Default/description
  libimobiledevice is a software library that talks the protocols to support
  iPhone®, iPod Touch®, iPad® and Apple TV® devices.
endef

define Package/libimobiledevice
  $(call Package/libimobiledevice/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libplist +libusbmuxd +libopenssl
  LICENSE:=LGPL-2.1-or-later
  LICENSE_FILES:=COPYING.LESSER
endef

define Package/libimobiledevice/description
  $(call Package/libimobiledevice/Default/description)
endef

define Package/libimobiledevice-utils
  $(call Package/libimobiledevice/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libimobiledevice
  LICENSE:=GPL-2.0-or-later
  ICENSE_FILES:=COPYING
endef

define Package/libimobiledevice-utils/description
  $(call Package/libimobiledevice/Default/description)
  This package contains the libimobiledevice utilities.
endef

CONFIGURE_ARGS += \
	--disable-static \
	--without-cython

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libimobiledevice $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libimobiledevice-1.0.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libimobiledevice-1.0.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libimobiledevice/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libimobiledevice-1.0.so.* $(1)/usr/lib/
endef

define Package/libimobiledevice-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/idevice* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libimobiledevice))
$(eval $(call BuildPackage,libimobiledevice-utils))
