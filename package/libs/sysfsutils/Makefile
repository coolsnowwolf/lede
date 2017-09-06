#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sysfsutils
PKG_VERSION:=2.1.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/linux-diag
PKG_HASH:=e865de2c1f559fff0d3fc936e660c0efaf7afe662064f2fb97ccad1ec28d208a
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING cmd/GPL lib/LGPL

PKG_FIXUP:=autoreconf

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libsysfs
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Filesystem
  TITLE:=Sysfs library
  URL:=http://linux-diag.sourceforge.net/Sysfsutils.html
endef

define Package/sysfsutils
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  DEPENDS:=+libsysfs
  TITLE:=System Utilities Based on Sysfs
  URL:=http://linux-diag.sourceforge.net/Sysfsutils.html
endef

define Package/libsysfs/description
The library's purpose is to provide a consistant and stable interface for
querying system device information exposed through sysfs.
endef

define Package/sysfsutils/description
A utility built upon libsysfs that lists devices by bus, class, and topology.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/sysfs $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsysfs.{a,so*,la} $(1)/usr/lib/
endef

define Package/libsysfs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsysfs.so* $(1)/usr/lib/
endef

define Package/sysfsutils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/systool $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libsysfs))
$(eval $(call BuildPackage,sysfsutils))
