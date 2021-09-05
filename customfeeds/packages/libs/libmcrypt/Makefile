#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmcrypt
PKG_VERSION:=2.5.8
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/mcrypt
PKG_HASH:=bf2f1671f44af88e66477db0982d5ecb5116a5c767b0a0d68acb34499d41b793

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING.LIB
PKG_CPE_ID:=cpe:/a:mcrypt:libmcrypt

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmcrypt
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Cryptographic library
  URL:=http://mcrypt.sourceforge.net/
endef

define Package/libmcrypt/description
	libmcrypt is a cryptographic library that conveniently brings
	together a variety of ciphers for convenient use.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmcrypt.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/libmcrypt-config $(1)/usr/bin/
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/libmcrypt-config
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/libmcrypt-config $(2)/bin/libmcrypt-config
endef

define Package/libmcrypt/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmcrypt.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmcrypt))
