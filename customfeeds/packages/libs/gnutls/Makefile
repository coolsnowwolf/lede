#
# Copyright (C) 2005-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gnutls
PKG_VERSION:=3.7.2
PKG_RELEASE:=$(AUTORELEASE)
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.gnupg.org/ftp/gcrypt/gnutls/v3.7
PKG_HASH:=646e6c5a9a185faa4cea796d378a1ba8e1148dbb197ca6605f95986a25af2752
PKG_FIXUP:=autoreconf gettext-version
PKG_MAINTAINER:=Nikos Mavrogiannopoulos <nmav@gnutls.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_CPE_ID:=cpe:/a:gnu:gnutls

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_LIBTOOL_PATHS:=. lib

PKG_CONFIG_DEPENDS:= \
	CONFIG_GNUTLS_ALPN \
	CONFIG_GNUTLS_ANON \
	CONFIG_GNUTLS_CRYPTODEV \
	CONFIG_GNUTLS_DTLS_SRTP \
	CONFIG_GNUTLS_EXT_LIBTASN1 \
	CONFIG_GNUTLS_HEARTBEAT \
	CONFIG_GNUTLS_OCSP \
	CONFIG_GNUTLS_PKCS11 \
	CONFIG_GNUTLS_PSK \
	CONFIG_GNUTLS_SRP \
	CONFIG_GNUTLS_TPM \
	CONFIG_LIBNETTLE_MINI \
	CONFIG_PACKAGE_libgnutls-dane \

include $(INCLUDE_DIR)/package.mk


define Package/gnutls/Default
  SUBMENU:=SSL
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU TLS
  URL:=http://www.gnutls.org/
endef

define Package/gnutls/Default/description
 GnuTLS is a secure communications library implementing the SSL, TLS
 and DTLS protocols and technologies around them. It provides a simple
 C language application programming interface (API) to access the secure
 communications protocols as well as APIs to parse and write X.509, PKCS12,
 OpenPGP and other required structures. It is aimed to be portable and
 efficient with focus on security and interoperability.
endef


define Package/certtool
$(call Package/gnutls/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE+= (certool utility)
  DEPENDS+= +libgnutls
endef

define Package/certtool/description
$(call Package/gnutls/Default/description)
 This package contains the GnuTLS certtool utility.
endef


define Package/gnutls-utils
$(call Package/gnutls/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE+= (utilities)
  DEPENDS+= +libgnutls +PACKAGE_libgnutls-dane:libgnutls-dane
endef

define Package/gnutls-utils/description
$(call Package/gnutls/Default/description)
 This package contains the GnuTLS gnutls-cli, gnutls-serv, psktool,
 and srptool utilities.
endef

define Package/libgnutls/config
	source "$(SOURCE)/Config.in"
endef

define Package/libgnutls
$(call Package/gnutls/Default)
  TITLE+= (library)
  DEPENDS+= +libnettle +!LIBNETTLE_MINI:libgmp +GNUTLS_EXT_LIBTASN1:libtasn1 +GNUTLS_PKCS11:p11-kit +GNUTLS_CRYPTODEV:kmod-cryptodev +libatomic
endef

define Package/libgnutls-dane
$(call Package/gnutls/Default)
  TITLE+= (libgnutls-dane library)
  DEPENDS:= +libgnutls +libunbound
endef

define Package/libgnutls/description
$(call Package/gnutls/Default/description)
 This package contains the GnuTLS shared library, needed by other programs.
endef

# We disable the configuration file (system-priority-file) because
# the use of configuration increases the non-shared memory used by
# the library and we don't provide an openwrt-specific configuration
# anyway.
CONFIGURE_ARGS+= \
	--enable-shared \
	--enable-static \
	--disable-doc \
	--disable-gcc-warnings \
	--disable-guile \
	--disable-rpath \
	--disable-seccomp-tests \
	--disable-tests \
	--disable-valgrind-tests \
	--disable-ssl2-support \
	--disable-ssl3-support \
	--enable-local-libopts \
	--without-idn \
	--with-default-trust-store-dir=/etc/ssl/certs/ \
	--with-included-unistring \
	--with-librt-prefix="$(LIBRT_ROOT_DIR)/" \
	--with-pic \
	--with-system-priority-file=""

ifneq ($(CONFIG_GNUTLS_EXT_LIBTASN1),y)
CONFIGURE_ARGS += --with-included-libtasn1
endif

ifneq ($(CONFIG_GNUTLS_PKCS11),y)
CONFIGURE_ARGS += --without-p11-kit
endif

ifeq ($(CONFIG_LIBNETTLE_MINI),y)
CONFIGURE_ARGS += --with-nettle-mini
endif

ifneq ($(CONFIG_GNUTLS_DTLS_SRTP),y)
CONFIGURE_ARGS += --disable-dtls-srtp-support
endif

ifneq ($(CONFIG_GNUTLS_ALPN),y)
CONFIGURE_ARGS += --disable-alpn-support
endif

ifneq ($(CONFIG_GNUTLS_HEARTBEAT),y)
CONFIGURE_ARGS += --disable-heartbeat-support
endif

ifneq ($(CONFIG_GNUTLS_SRP),y)
CONFIGURE_ARGS += --disable-srp-authentication
endif

ifneq ($(CONFIG_GNUTLS_PSK),y)
CONFIGURE_ARGS += --disable-psk-authentication
endif

ifneq ($(CONFIG_GNUTLS_ANON),y)
CONFIGURE_ARGS += --disable-anon-authentication
endif

ifneq ($(CONFIG_GNUTLS_OCSP),y)
CONFIGURE_ARGS += --disable-ocsp
endif

ifneq ($(CONFIG_GNUTLS_TPM),y)
CONFIGURE_ARGS += --without-tpm
endif

ifeq ($(CONFIG_GNUTLS_CRYPTODEV),y)
CONFIGURE_ARGS += --enable-cryptodev
endif

ifeq ($(CONFIG_PACKAGE_libgnutls-dane),)
CONFIGURE_ARGS += --disable-libdane
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/gnutls \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig/
endef


define Package/certtool/conffiles
/etc/gnutls/certtool.cfg
endef

define Package/certtool/install
	$(INSTALL_DIR) $(1)/etc/gnutls
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/doc/certtool.cfg $(1)/etc/gnutls/
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/certtool $(1)/usr/bin/
endef


define Package/gnutls-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
ifeq ($(CONFIG_GNUTLS_OCSP),y)
ifeq ($(CONFIG_GNUTLS_ANON),y)
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/gnutls-{cli,serv} \
		$(1)/usr/bin/
endif
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/ocsptool \
		$(1)/usr/bin/
endif
ifeq ($(CONFIG_GNUTLS_SRP),y)
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/srptool \
		$(1)/usr/bin/
endif
ifeq ($(CONFIG_GNUTLS_PSK),y)
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/psktool \
		$(1)/usr/bin/
endif
ifeq ($(CONFIG_GNUTLS_PKCS11),y)
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/p11tool \
		$(1)/usr/bin/
endif
ifeq ($(CONFIG_GNUTLS_TPM),y)
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/tpmtool \
		$(1)/usr/bin/
endif
endef


define Package/libgnutls/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgnutls.so.* $(1)/usr/lib/
endef

define Package/libgnutls-dane/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgnutls-dane.so.* $(1)/usr/lib/
endef


$(eval $(call BuildPackage,certtool))
$(eval $(call BuildPackage,gnutls-utils))
$(eval $(call BuildPackage,libgnutls))
$(eval $(call BuildPackage,libgnutls-dane))
