#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wolfssl
PKG_VERSION:=3.10.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://www.wolfssl.com/
PKG_HASH:=66f7f2a8b8ee37d6b4beab3cb0dcb6a6980fd4674373bfd3bf1214b9d0d2c02e

PKG_FIXUP:=libtool
PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk

define Package/libcyassl
  SECTION:=libs
  SUBMENU:=SSL
  CATEGORY:=Libraries
  TITLE:=CyaSSL library
  URL:=http://www.wolfssl.com/
  MENU:=1
endef

define Package/libcyassl/description
CyaSSL is an SSL library optimized for small footprint, both on disk and for
memory use.
endef

define Package/libcyassl/config
	source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += $(FPIC)

# --enable-stunnel needed for OpenSSL API compatibility bits
CONFIGURE_ARGS += \
	--enable-opensslextra \
	--enable-sni \
	--enable-stunnel \
	--disable-examples

ifeq ($(CONFIG_IPV6),y)
CONFIGURE_ARGS += \
        --enable-ipv6
endif

ifeq ($(CONFIG_CYASSL_HAS_AES_CCM),y)
CONFIGURE_ARGS += \
	--enable-aesccm
endif

ifeq ($(CONFIG_CYASSL_HAS_AES_GCM),y)
CONFIGURE_ARGS += \
	--enable-aesgcm
endif

ifeq ($(CONFIG_CYASSL_HAS_CHACHA),y)
CONFIGURE_ARGS += \
	--enable-chacha
endif

ifeq ($(CONFIG_CYASSL_HAS_ECC),y)
CONFIGURE_ARGS += \
	--enable-ecc \
	--enable-supportedcurves
endif

ifeq ($(CONFIG_CYASSL_HAS_DH),y)
CONFIGURE_ARGS += \
	--enable-dh
endif

ifeq ($(CONFIG_CYASSL_HAS_ARC4),n)
CONFIGURE_ARGS += \
	--disable-arc4
endif

ifeq ($(CONFIG_CYASSL_HAS_DES3),y)
CONFIGURE_ARGS += \
	--disable-des3
endif

ifeq ($(CONFIG_CYASSL_HAS_PSK),y)
CONFIGURE_ARGS += \
	--enable-psk
endif

ifeq ($(CONFIG_CYASSL_HAS_DTLS),y)
CONFIGURE_ARGS += \
	--enable-dtls
endif

ifeq ($(CONFIG_CYASSL_HAS_ECC25519),y)
CONFIGURE_ARGS += \
	--enable-ecc25519
endif

ifeq ($(CONFIG_CYASSL_HAS_POLY1305),y)
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

define Package/libcyassl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwolfssl.so* $(1)/usr/lib/
	ln -s libwolfssl.so $(1)/usr/lib/libcyassl.so
endef

$(eval $(call BuildPackage,libcyassl))
