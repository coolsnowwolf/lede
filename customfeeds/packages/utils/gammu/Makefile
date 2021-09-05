#
# Copyright (C) 2006-2015 OpenWrt.org
#		2014-2015 Vitaly Protsko
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gammu
PKG_VERSION:=1.42.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://dl.cihar.com/gammu/releases
PKG_HASH:=d8f152314d7e4d3d643610d742845e0a016ce97c234ad4b1151574e1b09651ee

PKG_MAINTAINER:=Vitaly Protsko <villy@sft.ru>
PKG_LICENCE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/gammu
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Cell phone/modem SMS and control tool
	URL:=https://wammu.eu/gammu/
	DEPENDS:=+libpthread +libcurl +glib2
	DEPENDS+=+PACKAGE_bluez-libs:bluez-libs
	DEPENDS+=+PACKAGE_libmariadb:libmariadb +PACKAGE_unixodbc:unixodbc +PACKAGE_libpq:libpq
	DEPENDS+=+PACKAGE_libusb-1.0:libusb-1.0
endef

CMAKE_OPTIONS += \
	-DDWITH_GettextLibs=OFF \
	-DWITH_Iconv=OFF \
	-DWITH_Libintl=OFF \
	-DWITH_LibDBI:BOOL=OFF

define Package/gammu/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gammu $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gammu-{smsd,smsd-inject,smsd-monitor} $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{Gammu*,gsmsd*} $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/gammu $(1)/etc/config/gammu
	$(INSTALL_BIN) ./files/gammu.init $(1)/etc/init.d/gammu
endef

define Package/gammu/conffiles
/etc/config/gammu
endef

$(eval $(call BuildPackage,gammu))
