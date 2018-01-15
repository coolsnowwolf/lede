#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=curl
PKG_VERSION:=7.57.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://dl.uxnr.de/mirror/curl/ \
	http://curl.mirror.anstey.ca/ \
	http://curl.askapache.com/download/ \
	https://curl.haxx.se/download/
PKG_HASH:=c92fe31a348eae079121b73884065e600c533493eb50f1f6cee9c48a3f454826

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:haxx:libcurl

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS:= \
  CONFIG_IPV6 \
  \
  CONFIG_LIBCURL_WOLFSSL \
  CONFIG_LIBCURL_GNUTLS \
  CONFIG_LIBCURL_OPENSSL \
  CONFIG_LIBCURL_MBEDTLS \
  CONFIG_LIBCURL_NOSSL \
  \
  CONFIG_LIBCURL_LIBIDN \
  CONFIG_LIBCURL_SSH2 \
  CONFIG_LIBCURL_ZLIB \
  \
  CONFIG_LIBCURL_DICT \
  CONFIG_LIBCURL_FILE \
  CONFIG_LIBCURL_FTP \
  CONFIG_LIBCURL_GOPHER \
  CONFIG_LIBCURL_HTTP \
  CONFIG_LIBCURL_IMAP \
  CONFIG_LIBCURL_LDAP \
  CONFIG_LIBCURL_LDAPS \
  CONFIG_LIBCURL_POP3 \
  CONFIG_LIBCURL_RTSP \
  CONFIG_LIBCURL_NO_RTSP \
  CONFIG_LIBCURL_SMB \
  CONFIG_LIBCURL_NO_SMB \
  CONFIG_LIBCURL_SMTP \
  CONFIG_LIBCURL_TELNET \
  CONFIG_LIBCURL_TFTP \
  CONFIG_LIBCURL_NGHTTP2 \
  \
  CONFIG_LIBCURL_COOKIES \
  CONFIG_LIBCURL_CRYPTO_AUTH \
  CONFIG_LIBCURL_LIBCURL_OPTION \
  CONFIG_LIBCURL_PROXY \
  CONFIG_LIBCURL_THREADED_RESOLVER \
  CONFIG_LIBCURL_TLS_SRP \
  CONFIG_LIBCURL_UNIX_SOCKETS \
  CONFIG_LIBCURL_VERBOSE \
  CONFIG_LIBCURL_NTLM

include $(INCLUDE_DIR)/package.mk

define Package/curl/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://curl.haxx.se/
  MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>
endef

define Package/curl
  $(call Package/curl/Default)
  SUBMENU:=File Transfer
  DEPENDS:=+libcurl
  TITLE:=A client-side URL transfer utility
endef

define Package/libcurl
  $(call Package/curl/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:= +LIBCURL_WOLFSSL:libwolfssl +LIBCURL_OPENSSL:libopenssl +LIBCURL_GNUTLS:libgnutls +LIBCURL_MBEDTLS:libmbedtls
  DEPENDS += +LIBCURL_ZLIB:zlib +LIBCURL_THREADED_RESOLVER:libpthread +LIBCURL_LDAP:libopenldap +LIBCURL_LIBIDN:libidn
  DEPENDS += +LIBCURL_SSH2:libssh2 +LIBCURL_NGHTTP2:libnghttp2
  TITLE:=A client-side URL transfer library
  MENU:=1
endef


define Package/libcurl/config
  source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += $(FPIC) -ffunction-sections -fdata-sections
TARGET_CPPFLAGS += $(if $(CONFIG_LIBCURL_NTLM),,-DCURL_DISABLE_NTLM)
TARGET_LDFLAGS += -Wl,--gc-sections

CONFIGURE_ARGS += \
	--disable-debug \
	--disable-ares \
	--enable-shared \
	--enable-static \
	--disable-manual \
	--without-nss \
	--without-libmetalink \
	--without-librtmp \
	\
	$(call autoconf_bool,CONFIG_IPV6,ipv6) \
	\
	$(if $(CONFIG_LIBCURL_WOLFSSL),--with-cyassl="$(STAGING_DIR)/usr" --without-ca-path --with-ca-bundle=/etc/ssl/certs/ca-certificates.crt,--without-cyassl) \
	$(if $(CONFIG_LIBCURL_GNUTLS),--with-gnutls="$(STAGING_DIR)/usr" --without-ca-bundle --with-ca-path=/etc/ssl/certs,--without-gnutls) \
	$(if $(CONFIG_LIBCURL_OPENSSL),--with-ssl="$(STAGING_DIR)/usr" --without-ca-bundle --with-ca-path=/etc/ssl/certs,--without-ssl) \
	$(if $(CONFIG_LIBCURL_MBEDTLS),--with-mbedtls="$(STAGING_DIR)/usr" --without-ca-path --with-ca-bundle=/etc/ssl/certs/ca-certificates.crt,--without-mbedtls) \
	\
	$(if $(CONFIG_LIBCURL_LIBIDN),--with-libidn="$(STAGING_DIR)/usr",--without-libidn) \
	$(if $(CONFIG_LIBCURL_SSH2),--with-libssh2="$(STAGING_DIR)/usr",--without-libssh2) \
	$(if $(CONFIG_LIBCURL_ZLIB),--with-zlib="$(STAGING_DIR)/usr",--without-zlib) \
	$(if $(CONFIG_LIBCURL_NGHTTP2),--with-nghttp2="$(STAGING_DIR)/usr",--without-nghttp2) \
	\
	$(call autoconf_bool,CONFIG_LIBCURL_DICT,dict) \
	$(call autoconf_bool,CONFIG_LIBCURL_FILE,file) \
	$(call autoconf_bool,CONFIG_LIBCURL_FTP,ftp) \
	$(call autoconf_bool,CONFIG_LIBCURL_GOPHER,gopher) \
	$(call autoconf_bool,CONFIG_LIBCURL_HTTP,http) \
	$(call autoconf_bool,CONFIG_LIBCURL_IMAP,imap) \
	$(call autoconf_bool,CONFIG_LIBCURL_LDAP,ldap) \
	$(call autoconf_bool,CONFIG_LIBCURL_LDAPS,ldaps) \
	$(call autoconf_bool,CONFIG_LIBCURL_POP3,pop3) \
	$(call autoconf_bool,CONFIG_LIBCURL_RTSP,rtsp) \
	$(call autoconf_bool,CONFIG_LIBCURL_SMB,smb) \
	$(call autoconf_bool,CONFIG_LIBCURL_SMTP,smtp) \
	$(call autoconf_bool,CONFIG_LIBCURL_TELNET,telnet) \
	$(call autoconf_bool,CONFIG_LIBCURL_TFTP,tftp) \
	\
	$(call autoconf_bool,CONFIG_LIBCURL_COOKIES,cookies) \
	$(call autoconf_bool,CONFIG_LIBCURL_CRYPTO_AUTH,crypto-auth) \
	$(call autoconf_bool,CONFIG_LIBCURL_LIBCURL_OPTION,libcurl-option) \
	$(call autoconf_bool,CONFIG_LIBCURL_PROXY,proxy) \
	$(call autoconf_bool,CONFIG_LIBCURL_THREADED_RESOLVER,threaded-resolver) \
	$(call autoconf_bool,CONFIG_LIBCURL_TLS_SRP,tls-srp) \
	$(call autoconf_bool,CONFIG_LIBCURL_UNIX_SOCKETS,unix-sockets) \
	$(call autoconf_bool,CONFIG_LIBCURL_VERBOSE,verbose) \

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" \
		install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin $(1)/usr/bin $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/curl-config $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/curl $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcurl.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libcurl.pc $(1)/usr/lib/pkgconfig/
	$(SED) 's,-L$$$${exec_prefix}/lib,,g' $(1)/usr/bin/curl-config
	[ -n "$(TARGET_LDFLAGS)" ] && $(SED) 's#$(TARGET_LDFLAGS)##g' $(1)/usr/lib/pkgconfig/libcurl.pc || true
	$(LN) $(STAGING_DIR)/usr/bin/curl-config $(2)/bin/
endef

define Package/curl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/curl $(1)/usr/bin/
endef

define Package/libcurl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcurl.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,curl))
$(eval $(call BuildPackage,libcurl))
