#
# Copyright (C) 2012-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libugpio
PKG_VERSION:=0.0.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/mhei/libugpio/releases/download/v$(PKG_VERSION)
PKG_HASH:=4e0ae6f9cd3d670eb5ba9595a24696a034fda3c7c1900aa9d2cae5a4cd9dab64

PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING.LESSER

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libugpio
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://github.com/mhei/libugpio
  TITLE:=Library for using sysfs gpio interface from C programs
  DEPENDS:=@GPIO_SUPPORT
endef

define Package/libugpio/description
  libugpio is a library to ease the use of linux kernel's sysfs
  gpio interface from C programs and/or other libraries.
endef

define Package/gpioctl-sysfs
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Tool for controlling gpio pins
  DEPENDS:=+libugpio
endef

define Package/gpioctl-sysfs/description
  Tool for controlling gpio pins using the sysfs api provided by the kernel.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ugpio $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libugpio.{so*,a} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libugpio.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libugpio/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libugpio.so.* $(1)/usr/lib/
endef

define Package/gpioctl-sysfs/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gpioctl $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libugpio))
$(eval $(call BuildPackage,gpioctl-sysfs))
