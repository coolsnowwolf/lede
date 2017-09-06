#
# Copyright (C) 2011-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=resolveip
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/resolveip
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Simple DNS resolver with configurable timeout
  MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
endef

define Package/resolveip/description
 This package contains the small resolveip utility which
 can be used by scripts to turn host names into numeric
 IP addresses. It supports IPv4 and IPv6 resolving and
 has a configurable timeout to guarantee a certain maximum
 runtime in case of slow or defunct DNS servers.
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -Wall \
		-o $(PKG_BUILD_DIR)/resolveip $(PKG_BUILD_DIR)/resolveip.c
endef

define Package/resolveip/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/resolveip $(1)/usr/bin/
endef

$(eval $(call BuildPackage,resolveip))
