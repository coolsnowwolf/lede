#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mailsend
PKG_VERSION:=1.19
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/muquit/mailsend/archive/$(PKG_VERSION)
PKG_HASH:=565ef6be26c58ef75065c5519eae8dd55bae3142928619696b28d242b73493f7

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYRIGHT

include $(INCLUDE_DIR)/package.mk

define Package/mailsend/default
	SECTION:=mail
	CATEGORY:=Mail
	TITLE:=A command-line mail sender
	URL:=https://github.com/muquit/mailsend
endef

define Package/mailsend-nossl
	$(call Package/mailsend/default)
	TITLE+= (without SSL)
	VARIANT:=nossl
endef

define Package/mailsend
	$(call Package/mailsend/default)
	TITLE+= (with SSL)
	DEPENDS:=+libopenssl
	VARIANT:=ssl
endef

ifeq ($(BUILD_VARIANT),ssl)
  CONFIGURE_ARGS+= --with-openssl=$(STAGING_DIR)/usr
endif

define Package/mailsend/description
  $(call Package/mailsend-nossl/description)
  .
  SSL supported is provided by OpenSSL.
endef

define Package/mailsend-nossl/description
  Mailsend is a simple command line program to send mail via SMTP protocol.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
endef

define Package/mailsend/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mailsend $(1)/usr/bin/
endef

Package/mailsend-nossl/install=$(Package/mailsend/install)

$(eval $(call BuildPackage,mailsend))
$(eval $(call BuildPackage,mailsend-nossl))
