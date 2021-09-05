#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cyrus-sasl
PKG_VERSION:=2.1.27
PKG_RELEASE:=3

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.cyrusimap.org/releases/
PKG_HASH:=26866b1549b00ffd020f188a43c258017fa1c382b3ddadd8201536f72efb05d5

PKG_LICENSE:=BSD-4c BSD
PKG_LICENSE_FILES:=COPYING cmulocal/COPYING saslauthd/COPYING
PKG_CPE_ID:=cpe:/a:cmu:cyrus-sasl

PKG_FIXUP:=autoreconf
PKG_MACRO_PATHS:=cmulocal config ../cmulocal ../config
PKG_AUTOMAKE_PATHS:=. saslauthd sasldb
PKG_REMOVE_FILES:=aclocal.m4 saslauthd/aclocal.m4 config/libtool.m4

include $(INCLUDE_DIR)/package.mk

define Package/libsasl2
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A general purpose authentication library
  URL:=http://asg.web.cmu.edu/sasl/
  DEPENDS:=+libopenssl
endef

TARGET_CFLAGS += $(FPIC)
CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-sample \
	--enable-staticdlopen \
	--disable-java \
	--disable-alwaystrue \
	--disable-checkapop \
	--enable-cram \
	--enable-digest \
	--without-auth-sasldb \
	--disable-otp \
	--disable-srp \
	--disable-srp-setpass \
	--disable-krb4 \
	--disable-gssapi \
	--disable-gss_mutexes \
	--enable-plain \
	--enable-anon \
	--disable-login \
	--disable-ntlm \
	--disable-sql \
	--disable-ldapdb \
	--without-dblib \
	--without-gdbm \
	--with-devrandom="/dev/urandom" \
	--without-pam \
	--without-saslauthd \
	--without-authdaemond \
	--without-pwcheck \
	--with-ipctype=unix \
	--with-openssl="$(STAGING_DIR)/usr" \
	--without-des \
	--without-opie \
	--without-ldap \
	--without-mysql \
	--without-pgsql \
	--without-sqlite \
	--without-rc4 \
	--without-dmalloc \
	--without-sfio \
	--disable-sample

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/include \
		CC="$(HOSTCC)" \
		LINK="$(HOSTCC) -o makemd5 -lc" \
		CFLAGS="" \
		CPPFLAGS="" \
		makemd5
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/sasl $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsasl2.{a,so*} $(1)/usr/lib/
	ln -sf libsasl2.a $(1)/usr/lib/libsasl.a
	ln -sf libsasl2.so $(1)/usr/lib/libsasl.so
	$(INSTALL_DIR) $(1)/usr/lib/sasl2
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/sasl2/lib*.{a,so*} $(1)/usr/lib/sasl2/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libsasl2.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libsasl2/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsasl2.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/sasl2
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/sasl2/lib*.so* $(1)/usr/lib/sasl2/
endef

$(eval $(call BuildPackage,libsasl2))
