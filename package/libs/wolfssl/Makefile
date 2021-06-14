#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wolfssl
PKG_VERSION:=4.7.0-stable
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/wolfSSL/wolfssl/archive/v$(PKG_VERSION)
PKG_HASH:=b0e740b31d4d877d540ad50cc539a8873fc41af02bd3091c4357b403f7106e31

PKG_FIXUP:=libtool
PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSING COPYING
PKG_MAINTAINER:=Eneas U de Queiroz <cotequeiroz@gmail.com>
PKG_CPE_ID:=cpe:/a:wolfssl:wolfssl

PKG_CONFIG_DEPENDS:=\
	CONFIG_WOLFSSL_HAS_AES_CCM CONFIG_WOLFSSL_HAS_AFALG \
	CONFIG_WOLFSSL_HAS_ARC4 CONFIG_WOLFSSL_HAS_CHACHA_POLY \
	CONFIG_WOLFSSL_HAS_DEVCRYPTO_AES CONFIG_WOLFSSL_HAS_DEVCRYPTO_FULL \
	CONFIG_WOLFSSL_HAS_DH CONFIG_WOLFSSL_HAS_DTLS \
	CONFIG_WOLFSSL_HAS_ECC25519 CONFIG_WOLFSSL_HAS_OCSP \
	CONFIG_WOLFSSL_HAS_SESSION_TICKET CONFIG_WOLFSSL_HAS_TLSV10 \
	CONFIG_WOLFSSL_HAS_TLSV13 CONFIG_WOLFSSL_HAS_WPAS CONFIG_WOLFSSL_HAS_CERTGEN \
	CONFIG_WOLFSSL_HAS_OPENVPN

include $(INCLUDE_DIR)/package.mk

define Package/libwolfssl
  SECTION:=libs
  SUBMENU:=SSL
  CATEGORY:=Libraries
  TITLE:=wolfSSL library
  URL:=http://www.wolfssl.com/
  MENU:=1
  PROVIDES:=libcyassl
  DEPENDS:=+WOLFSSL_HAS_DEVCRYPTO:kmod-cryptodev +WOLFSSL_HAS_AFALG:kmod-crypto-user
  ABI_VERSION:=24
endef

define Package/libwolfssl/description
wolfSSL (formerly CyaSSL) is an SSL library optimized for small
footprint, both on disk and for memory use.
endef

define Package/libwolfssl/config
	source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += $(FPIC) -DFP_MAX_BITS=8192 -fomit-frame-pointer -flto
TARGET_LDFLAGS += -flto

# --enable-stunnel needed for OpenSSL API compatibility bits
CONFIGURE_ARGS += \
	--enable-lighty \
	--enable-opensslall \
	--enable-opensslextra \
	--enable-sni \
	--enable-stunnel \
	--disable-crypttests \
	--disable-examples \
	--disable-jobserver \
	--$(if $(CONFIG_IPV6),enable,disable)-ipv6 \
	--$(if $(CONFIG_WOLFSSL_HAS_AES_CCM),enable,disable)-aesccm \
	--$(if $(CONFIG_WOLFSSL_HAS_CERTGEN),enable,disable)-certgen \
	--$(if $(CONFIG_WOLFSSL_HAS_CHACHA_POLY),enable,disable)-chacha \
	--$(if $(CONFIG_WOLFSSL_HAS_CHACHA_POLY),enable,disable)-poly1305 \
	--$(if $(CONFIG_WOLFSSL_HAS_DH),enable,disable)-dh \
	--$(if $(CONFIG_WOLFSSL_HAS_ARC4),enable,disable)-arc4 \
	--$(if $(CONFIG_WOLFSSL_HAS_TLSV10),enable,disable)-tlsv10 \
	--$(if $(CONFIG_WOLFSSL_HAS_TLSV13),enable,disable)-tls13 \
	--$(if $(CONFIG_WOLFSSL_HAS_SESSION_TICKET),enable,disable)-session-ticket \
	--$(if $(CONFIG_WOLFSSL_HAS_DTLS),enable,disable)-dtls \
	--$(if $(CONFIG_WOLFSSL_HAS_ECC25519),enable,disable)-curve25519 \
	--$(if $(CONFIG_WOLFSSL_HAS_AFALG),enable,disable)-afalg \
	--$(if $(CONFIG_WOLFSSL_HAS_OPENVPN),enable,disable)-openvpn \
	--enable-devcrypto=$(if $(CONFIG_WOLFSSL_HAS_DEVCRYPTO_CBC),cbc\
			  ,$(if $(CONFIG_WOLFSSL_HAS_DEVCRYPTO_AES),aes\
			  ,$(if $(CONFIG_WOLFSSL_HAS_DEVCRYPTO_FULL),yes,no)))

ifeq ($(CONFIG_WOLFSSL_HAS_OCSP),y)
CONFIGURE_ARGS += \
	--enable-ocsp --enable-ocspstapling --enable-ocspstapling2
endif

ifeq ($(CONFIG_WOLFSSL_HAS_WPAS),y)
CONFIGURE_ARGS += \
	--enable-wpas --enable-sha512 --enable-fortress --enable-fastmath
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwolfssl.{so*,la} $(1)/usr/lib/
	ln -s libwolfssl.so $(1)/usr/lib/libcyassl.so
	ln -s libwolfssl.la $(1)/usr/lib/libcyassl.la

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig
endef

define Package/libwolfssl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwolfssl.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libwolfssl))
