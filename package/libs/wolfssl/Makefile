#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wolfssl
PKG_VERSION:=3.15.3-stable
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
# PKG_SOURCE_URL:=https://www.wolfssl.com/
PKG_SOURCE_URL:=https://github.com/wolfSSL/wolfssl/archive/v$(PKG_VERSION)
PKG_HASH:=dc97c07a7667b39a890e14f4b4a209f51524a4cabee7adb6c80822ee78c1f62a

PKG_FIXUP:=libtool
PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0+
PKG_CPE_ID:=cpe:/a:yassl:cyassl

PKG_CONFIG_DEPENDS:=\
	CONFIG_WOLFSSL_HAS_AES_CCM CONFIG_WOLFSSL_HAS_AES_GCM \
	CONFIG_WOLFSSL_HAS_ARC4 CONFIG_WOLFSSL_HAS_CHACHA \
	CONFIG_WOLFSSL_HAS_DES3 CONFIG_WOLFSSL_HAS_DH CONFIG_WOLFSSL_HAS_DTLS \
	CONFIG_WOLFSSL_HAS_ECC CONFIG_WOLFSSL_HAS_ECC25519 \
	CONFIG_WOLFSSL_HAS_OCSP CONFIG_WOLFSSL_HAS_POLY_1305 \
	CONFIG_WOLFSSL_HAS_PSK CONFIG_WOLFSSL_HAS_SESSION_TICKET \
	CONFIG_WOLFSSL_HAS_WPAS

include $(INCLUDE_DIR)/package.mk

define Package/libwolfssl
  SECTION:=libs
  SUBMENU:=SSL
  CATEGORY:=Libraries
  TITLE:=wolfSSL library
  URL:=http://www.wolfssl.com/
  MENU:=1
  PROVIDES:=libcyassl
endef

define Package/libwolfssl/description
wolfSSL (formerly CyaSSL) is an SSL library optimized for small
footprint, both on disk and for memory use.
endef

define Package/libwolfssl/config
	source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += $(FPIC)

# --enable-stunnel needed for OpenSSL API compatibility bits
CONFIGURE_ARGS += \
	--enable-opensslextra \
	--enable-sni \
	--enable-stunnel \
	--disable-examples \
	--disable-leanpsk \
	--disable-leantls \

ifeq ($(CONFIG_IPV6),y)
CONFIGURE_ARGS += \
        --enable-ipv6
endif

ifeq ($(CONFIG_WOLFSSL_HAS_AES_CCM),y)
CONFIGURE_ARGS += \
	--enable-aesccm
endif

ifneq ($(CONFIG_WOLFSSL_HAS_AES_GCM),y)
CONFIGURE_ARGS += \
	--disable-aesgcm
endif

ifneq ($(CONFIG_WOLFSSL_HAS_CHACHA),y)
CONFIGURE_ARGS += \
	--disable-chacha
endif

ifeq ($(CONFIG_WOLFSSL_HAS_ECC),y)
CONFIGURE_ARGS += \
	--enable-ecc \
	--enable-supportedcurves
endif

ifeq ($(CONFIG_WOLFSSL_HAS_DH),y)
CONFIGURE_ARGS += \
	--enable-dh
endif

ifneq ($(CONFIG_WOLFSSL_HAS_ARC4),y)
CONFIGURE_ARGS += \
	--disable-arc4
else
CONFIGURE_ARGS += \
	--enable-arc4
endif

ifneq ($(CONFIG_WOLFSSL_HAS_DES3),y)
CONFIGURE_ARGS += \
	--disable-des3
else
CONFIGURE_ARGS += \
	--enable-des3
endif

ifeq ($(CONFIG_WOLFSSL_HAS_PSK),y)
CONFIGURE_ARGS += \
	--enable-psk
endif

ifeq ($(CONFIG_WOLFSSL_HAS_SESSION_TICKET),y)
CONFIGURE_ARGS += \
	--enable-session-ticket
endif

ifeq ($(CONFIG_WOLFSSL_HAS_DTLS),y)
CONFIGURE_ARGS += \
	--enable-dtls
endif

ifeq ($(CONFIG_WOLFSSL_HAS_OCSP),y)
CONFIGURE_ARGS += \
	--enable-ocsp --enable-ocspstapling --enable-ocspstapling2
endif

ifeq ($(CONFIG_WOLFSSL_HAS_WPAS),y)
CONFIGURE_ARGS += \
	--enable-wpas --enable-sha512 --enable-fortress --enable-fastmath
endif

ifeq ($(CONFIG_WOLFSSL_HAS_ECC25519),y)
CONFIGURE_ARGS += \
	--enable-curve25519
endif

ifneq ($(CONFIG_WOLFSSL_HAS_POLY1305),y)
CONFIGURE_ARGS += \
	--enable-poly1305
endif

#ifneq ($(CONFIG_TARGET_x86),)
#	CONFIGURE_ARGS += --enable-intelasm
#endif
#ifneq ($(CONFIG_TARGET_x86_64),)
#	CONFIGURE_ARGS += --enable-intelasm
#endif

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
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwolfssl.so* $(1)/usr/lib/
	ln -s libwolfssl.so $(1)/usr/lib/libcyassl.so
endef

$(eval $(call BuildPackage,libwolfssl))
