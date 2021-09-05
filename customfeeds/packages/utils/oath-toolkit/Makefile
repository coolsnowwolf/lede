#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=oath-toolkit
PKG_VERSION:=2.6.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SAVANNAH/oath-toolkit
PKG_HASH:=d207120c7e7fdd540142d04ca06d83fb3277c8f2fb794a74535d04b2aa0ec219

PKG_MAINTAINER:=Fam Zheng <fam@euphon.net>
PKG_LICENSE:=LGPL-2.0-or-later GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:nongnu:oath_toolkit

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

CONFIGURE_ARGS += \
	--disable-xmltest \
	--disable-pskc

define Package/oath-toolkit
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Toolkit for building one-time password authentication
  URL:=http://www.nongnu.org/oath-toolkit/index.html
  DEPENDS:=
endef

define Package/oath-toolkit/description
	The OATH Toolkit provide components for building one-time password
	authentication systems. It contains shared libraries, command line
	tools and a PAM module. Supported technologies include the event-based
	HOTP algorithm (RFC4226) and the time-based TOTP algorithm (RFC6238).
endef

define Package/oath-toolkit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/oathtool $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboath.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,oath-toolkit))
