#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hdparm
PKG_VERSION:=9.60
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=8397739c73e44d5ab96c4aef28fa9c0147276d53a1b5657ce04c4565cf6635cc

PKG_MAINTAINER:=
PKG_LICENSE:=BSD-Style Open Source License
PKG_LICENSE_FILES:=LICENSE.TXT

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/hdparm
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Disc
  TITLE:=Hard disk drive configuration utilitity
  URL:=https://sourceforge.net/projects/hdparm/
endef

define Package/hdparm/description
 get/set SATA/IDE device parameters
endef

define Package/hdparm/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/hdparm $(1)/sbin
endef

$(eval $(call BuildPackage,hdparm))
