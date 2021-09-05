#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pps-tools
PKG_VERSION:=1.0.2
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://codeload.github.com/redlab-i/pps-tools/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=1a7efd66152e5439b69143f1f380b40ac5decbbbef516b37a017410b8ba7dff4

PKG_MAINTAINER:=Zoltan HERPAI <wigyori@uid0.hu>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/pps-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=PPS-tools
endef

define Package/pps-tools/description
  Userland tools for GPS and DCF-77 Clock synchronization. PPS support in Kernel must be enabled.
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/sys
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/timepps.h $(1)/usr/include/sys/
endef

define Package/pps-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ppsctl $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ppstest $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ppswatch $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,pps-tools))
