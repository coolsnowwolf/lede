#
# Copyright (C) 2012-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libusbmuxd
PKG_VERSION:=2.0.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.libimobiledevice.org/downloads
PKG_HASH:=cc6a808553da4efa9fa5638be256d5ae020498795d9d260d280b87074e799b20

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_CPE_ID:=cpe:/a:libimobiledevice:libusbmuxd

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libusbmuxd/Default
  TITLE:=USB multiplexing daemon
  URL:=https://www.libimobiledevice.org/
  SUBMENU:=libimobiledevice
endef

define Package/libusbmuxd/Default/description
  This daemon is in charge of multiplexing connections over USB to an iPhone or
  iPod touch. To users, it means you can sync your music, contacts, photos, etc.
  over USB. To developers, it means you can connect to any listening localhost
  socket on the device. usbmuxd is not used for tethering data transfer, which
  uses a dedicated USB interface as a virtual network device.
endef

define Package/libusbmuxd
  $(call Package/libusbmuxd/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= library
  DEPENDS:=+libplist +libpthread +libxml2 +zlib
  PKG_LICENSE:=LGPL-2.1-or-later
  PKG_LICENSE_FILES:=COPYING
endef

define Package/libusbmuxd/description
  $(call Package/libusbmuxd/Default/description)
  This package contains the libusbmuxd shared library.
endef

define Package/libusbmuxd-utils
  $(call Package/libusbmuxd/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= utilies
  DEPENDS:=+libusbmuxd
  LICENSE:=GPL-2.0-or-later
endef

define Package/libusbmuxd-utils/description
  $(call Package/libusbmuxd/Default/description)
  This package contains the libusbmuxd utilities.
endef

CONFIGURE_ARGS += \
	--disable-static

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusbmuxd-2.0.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libusbmuxd-2.0.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libusbmuxd/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libusbmuxd-2.0.so.* $(1)/usr/lib/
endef

define Package/libusbmuxd-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/iproxy $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/inetcat $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libusbmuxd))
$(eval $(call BuildPackage,libusbmuxd-utils))
