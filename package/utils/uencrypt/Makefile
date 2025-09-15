# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2022 Eneas Ulir de Queiroz

include $(TOPDIR)/rules.mk

PKG_NAME:=uencrypt
PKG_RELEASE:=2

PKG_FLAGS:=nonshared
PKG_LICENSE:=GPL-2.0-or-later
PKG_MAINTAINER:=Eneas U de Queiroz <cotequeiroz@gmail.com>
PKG_CONFIG_DEPENDS:=\
	CONFIG_UENCRYPT_OPENSSL \
	CONFIG_UENCRYPT_WOLFSSL

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_INSTALL:=1
CMAKE_OPTIONS+=$(if $(CONFIG_UENCRYPT_WOLFSSL),-DUSE_WOLFSSL=1)

define Package/uencrypt
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Decryption utility for Arcadyan WG4xx223 and TP-Link Deco S4
  DEPENDS:=+UENCRYPT_WOLFSSL:libwolfssl +UENCRYPT_OPENSSL:libopenssl
endef

define Package/uencrypt/description
  This is a small encrypton/decryption program. It defaults
  to AES-128-CBC, but supports any encryption provided by
  the available openssl/wolfssl library. Even though it can
  be used for regular encryption and decryption operations,
  it is included here to unencrypt the configuration from mtd
  on Arcadyan WG430223/WG443223 and TP-Link Deco S4 routers
endef

define Package/uencrypt/config
  if PACKAGE_uencrypt
    choice
	prompt "Crypto provider"
	default UENCRYPT_OPENSSL

	config UENCRYPT_OPENSSL
	bool "OpenSSL"

	config UENCRYPT_WOLFSSL
	bool "wolfSSL"
    endchoice
  endif
endef

define Package/uencrypt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/uencrypt $(1)/usr/bin
endef

$(eval $(call BuildPackage,uencrypt))
