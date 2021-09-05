#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=loudmouth
PKG_VERSION:=1.5.4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/mcabber/loudmouth/tar.gz/$(PKG_VERSION)?
PKG_HASH:=d1f66d479bb9a2794c52e997a9db9271f7fd004e264fe1ff4302607deb083375

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/loudmouth
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2 +libopenssl +PACKAGE_krb5-libs:krb5-libs
  TITLE:=loudmouth
  URL:=https://github.com/mcabber/loudmouth
endef

define Package/loudmouth/description
  Lightweight and easy-to-use C library for programming with the Jabber protocol
endef

CONFIGURE_ARGS += \
	--disable-debug \
	--without-compile-warnings \
	--without-idn \
	--with-ssl=openssl

CONFIGURE_VARS += \
	ac_cv_lib_crypto_BIO_f_base64=yes \
	ac_cv_lib_ssl_SSL_new=yes

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/loudmouth-1.0/ \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/loudmouth/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,loudmouth))
