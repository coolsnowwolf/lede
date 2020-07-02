#
# Copyright (C) 2011-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=usbreset
PKG_RELEASE:=5

include $(INCLUDE_DIR)/package.mk

define Package/usbreset
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Utility to send a USB port reset to a USB device
  MAINTAINER:=Jo-Philipp Wich <xm@subsignal.org>
endef

define Package/usbreset/description
 This package contains the small usbreset utility which
 can be used to send a USB port reset to a USB device -
 useful for debugging or to force re-detection of particular
 devices.
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -Wall \
		-o $(PKG_BUILD_DIR)/usbreset $(PKG_BUILD_DIR)/usbreset.c
endef

define Package/usbreset/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usbreset $(1)/usr/bin/
endef

$(eval $(call BuildPackage,usbreset))
