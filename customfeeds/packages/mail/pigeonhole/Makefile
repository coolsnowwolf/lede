#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dovecot-pigeonhole
PKG_VERSION_PLUGIN:=0.5.14
PKG_VERSION_DOVECOT:=$(shell make --no-print-directory -C ../dovecot/ val.PKG_VERSION V=s)
PKG_VERSION:=$(PKG_VERSION_DOVECOT)-$(PKG_VERSION_PLUGIN)
PKG_RELEASE:=$(AUTORELEASE)

DOVECOT_VERSION:=2.3

PKG_SOURCE:=dovecot-$(DOVECOT_VERSION)-pigeonhole-$(PKG_VERSION_PLUGIN).tar.gz
PKG_SOURCE_URL:=https://pigeonhole.dovecot.org/releases/$(DOVECOT_VERSION)
PKG_HASH:=68ca0f78a3caa6b090a469f45c395c44cf16da8fcb3345755b1ca436c9ffb2d2

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING COPYING.LGPL
PKG_CPE_ID:=cpe:/a:dovecot:pigeonhole

PKG_BUILD_DIR:=$(BUILD_DIR)/dovecot-$(DOVECOT_VERSION)-pigeonhole-$(PKG_VERSION_PLUGIN)
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/dovecot-pigeonhole
  SECTION:=mail
  CATEGORY:=Mail
  DEPENDS:=+dovecot
  EXTRA_DEPENDS:=dovecot (>= $(PKG_VERSION_DOVECOT))
  TITLE:=Mail filtering facilities for Dovecot
  URL:=https://wiki2.dovecot.org/Pigeonhole
endef

define Package/dovecot-pigeonhole/description
  Pigeonhole provides mail filtering facilities for Dovecot using the Sieve
  (RFC 5228) language.
endef

CONFIGURE_ARGS += \
  --with-dovecot=$(STAGING_DIR)/usr/lib/dovecot/

CONFIGURE_VARS += \
  LDFLAGS="$(TARGET_LDFLAGS) -L$(STAGING_DIR)/usr/lib/dovecot/" \
  CPPFLAGS="$(TARGET_CPPFLAGS) -I$(STAGING_DIR)/usr/include/dovecot/"

define Package/dovecot-pigeonhole/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/dovecot/ $(1)/usr/libexec/dovecot/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/dovecot/* $(1)/usr/lib/dovecot/
	$(CP) $(PKG_INSTALL_DIR)/usr/libexec/dovecot/* $(1)/usr/libexec/dovecot/
	find $(1)/usr/lib/dovecot/ -name "*.a" -o -name "*.la" | xargs rm
endef

$(eval $(call BuildPackage,dovecot-pigeonhole))
