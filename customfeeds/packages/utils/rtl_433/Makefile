#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl_433
PKG_VERSION:=20.11
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/merbanan/rtl_433/tar.gz/$(PKG_VERSION)?
PKG_HASH:=12a5cb7a733ba352467522c704d5b685aa6137582dc18aaa444d4891c29ee839

PKG_MAINTAINER:=Jasper Scholte <NightNL@outlook.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/rtl_433
  TITLE:=Realtek RTL2832 based 433.92MHz generic data receiver
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+librtlsdr +rtl-sdr +libtool-bin
  URL:=https://github.com/merbanan/rtl_433
endef

define Package/rtl-sdr/description
  rtl_433 turns your Realtek RTL2832 based DVB dongle into a 433.92MHz generic data receiver.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/rtl_433.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/rtl_433_devices.h $(1)/usr/include
endef

define Package/rtl_433/install
	$(INSTALL_DIR) $(1)/etc/rtl_433
	$(CP) $(PKG_INSTALL_DIR)/usr/etc/rtl_433/*.conf $(1)/etc/rtl_433
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/rtl_433 $(1)/usr/bin
endef

$(eval $(call BuildPackage,rtl_433))
