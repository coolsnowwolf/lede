#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ngircd
PKG_VERSION:=26.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://ngircd.barton.de/pub/ngircd
PKG_HASH:=55c16fd26009f6fc6a007df4efac87a02e122f680612cda1ce26e17a18d86254

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:ngircd:ngircd

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ngircd/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Instant Messaging
  DEPENDS:=+zlib
  TITLE:=Next Generation IRC Server
  URL:=https://ngircd.barton.de
endef

define Package/ngircd/Default/description
  ngIRCd is an Open Source server for the Internet Relay Chat (IRC), which
  is developed and published under the terms of the GNU General Public Licence.
  ngIRCd means "next generation IRC daemon", it's written from scratch and
  not deduced from the "grandfather of IRC daemons", the daemon of the IRCNet.
endef

define Package/ngircd
  $(call Package/ngircd/Default)
  TITLE+= (with OpenSSL support)
  DEPENDS+= +libopenssl
  VARIANT:=ssl
endef

define Package/ngircd/conffiles
/etc/ngircd.conf
endef

define Package/ngircd/description
  $(call Package/ngircd/Default/description)
  This package is built with OpenSSL support.
endef

define Package/ngircd-nossl
$(call Package/ngircd/Default)
  TITLE+= (without OpenSSL support)
  VARIANT:=nossl
endef

Package/ngircd-nossl/conffiles = $(Package/ngircd/conffiles)

define Package/ngircd-nossl/description
  $(call Package/ngircd/Default/description)
  This package is built without OpenSSL support.
endef

ifeq ($(CONFIG_IPV6),y)
	CONFIGURE_ARGS += \
		--enable-ipv6
endif

ifeq ($(BUILD_VARIANT),ssl)
	CONFIGURE_ARGS += \
		--with-openssl="$(STAGING_DIR)/usr"
endif

ifeq ($(BUILD_VARIANT),nossl)
	CONFIGURE_ARGS += \
		--without-openssl
endif

# The test for HAVE_WORKING_GETADDRINFO fails
# when cross-compiling.
#
define Build/Configure
	$(call Build/Configure/Default)
	$(SED) 's/\/\* #undef HAVE_WORKING_GETADDRINFO \*\//#define HAVE_WORKING_GETADDRINFO 1/' $(PKG_BUILD_DIR)/src/config.h
endef

define Package/ngircd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ngircd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/ngircd.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ngircd.init $(1)/etc/init.d/ngircd
endef

Package/ngircd-nossl/install = $(Package/ngircd/install)

$(eval $(call BuildPackage,ngircd))
$(eval $(call BuildPackage,ngircd-nossl))
