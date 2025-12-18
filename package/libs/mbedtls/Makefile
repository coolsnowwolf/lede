#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mbedtls
PKG_VERSION:=3.6.5
PKG_RELEASE:=1
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL=https://github.com/Mbed-TLS/$(PKG_NAME)/releases/download/$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=4a11f1777bb95bf4ad96721cac945a26e04bf19f57d905f241fe77ebeddf46d8

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:arm:mbed_tls

MBEDTLS_BUILD_OPTS_CURVES= \
  CONFIG_MBEDTLS_ECP_DP_SECP192R1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_SECP224R1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_SECP256R1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_SECP384R1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_SECP521R1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_SECP192K1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_SECP224K1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_SECP256K1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_BP256R1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_BP384R1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_BP512R1_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_CURVE25519_ENABLED \
  CONFIG_MBEDTLS_ECP_DP_CURVE448_ENABLED

MBEDTLS_BUILD_OPTS_CIPHERS= \
  CONFIG_MBEDTLS_AES_C \
  CONFIG_MBEDTLS_ARIA_C \
  CONFIG_MBEDTLS_CAMELLIA_C \
  CONFIG_MBEDTLS_CCM_C \
  CONFIG_MBEDTLS_CMAC_C \
  CONFIG_MBEDTLS_DES_C \
  CONFIG_MBEDTLS_GCM_C \
  CONFIG_MBEDTLS_KEY_EXCHANGE_PSK_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_RSA_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED \
  CONFIG_MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED \
  CONFIG_MBEDTLS_NIST_KW_C \
  CONFIG_MBEDTLS_RIPEMD160_C \
  CONFIG_MBEDTLS_RSA_NO_CRT \
  CONFIG_MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_ENABLED \
  CONFIG_MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_EPHEMERAL_ENABLED \
  CONFIG_MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_EPHEMERAL_ENABLED

MBEDTLS_BUILD_OPTS= \
  $(MBEDTLS_BUILD_OPTS_CURVES) \
  $(MBEDTLS_BUILD_OPTS_CIPHERS) \
  CONFIG_MBEDTLS_CIPHER_MODE_OFB \
  CONFIG_MBEDTLS_CIPHER_MODE_XTS \
  CONFIG_MBEDTLS_DEBUG_C \
  CONFIG_MBEDTLS_ENTROPY_FORCE_SHA256 \
  CONFIG_MBEDTLS_HKDF_C \
  CONFIG_MBEDTLS_PLATFORM_C \
  CONFIG_MBEDTLS_SELF_TEST \
  CONFIG_MBEDTLS_SSL_RENEGOTIATION \
  CONFIG_MBEDTLS_THREADING_C \
  CONFIG_MBEDTLS_THREADING_PTHREAD \
  CONFIG_MBEDTLS_VERSION_C \
  CONFIG_MBEDTLS_VERSION_FEATURES \
  CONFIG_MBEDTLS_PSA_CRYPTO_CLIENT \
  CONFIG_MBEDTLS_DEPRECATED_WARNING \
  CONFIG_MBEDTLS_SSL_PROTO_TLS1_2 \
  CONFIG_MBEDTLS_SSL_PROTO_TLS1_3 \
  CONFIG_MBEDTLS_SSL_TLS1_3_COMPATIBILITY_MODE

PKG_CONFIG_DEPENDS := $(MBEDTLS_BUILD_OPTS)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mbedtls/Default
  TITLE:=Embedded SSL
  URL:=https://www.trustedfirmware.org/projects/mbed-tls/
endef

define Package/mbedtls/Default/description
The aim of the mbedtls project is to provide a quality, open-source
cryptographic library written in C and targeted at embedded systems.
endef

define Package/libmbedtls
$(call Package/mbedtls/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=SSL
  TITLE+= (library)
  ABI_VERSION:=21
  MENU:=1
endef

define Package/libmbedtls/config
	source "$(SOURCE)/Config.in"
endef

define Package/mbedtls-util
$(call Package/mbedtls/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+= (utilities)
  DEPENDS:=+libmbedtls
endef

define Package/libmbedtls/description
$(call Package/mbedtls/Default/description)
This package contains the mbedtls library.
endef

define Package/mbedtls-util/description
$(call Package/mbedtls/Default/description)
This package contains mbedtls helper programs for private key and
CSR generation (gen_key, cert_req)
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS))

CMAKE_OPTIONS += \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DUSE_SHARED_MBEDTLS_LIBRARY:Bool=ON \
	-DENABLE_TESTING:Bool=OFF \
	-DENABLE_PROGRAMS:Bool=ON

define Build/Prepare
       $(call Build/Prepare/Default)

       $(if $(strip $(foreach opt,$(MBEDTLS_BUILD_OPTS),$($(opt)))),
	 $(foreach opt,$(MBEDTLS_BUILD_OPTS),
	 $(PKG_BUILD_DIR)/scripts/config.py \
	 -f $(PKG_BUILD_DIR)/include/mbedtls/mbedtls_config.h \
	 $(if $($(opt)),set,unset) $(patsubst CONFIG_%,%,$(opt))),)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mbedtls \
		$(PKG_INSTALL_DIR)/usr/include/psa \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/cmake $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mbedcrypto.pc \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mbedtls.pc \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mbedx509.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libmbedtls/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so.* $(1)/usr/lib/
endef

define Package/mbedtls-util/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gen_key $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cert_req $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libmbedtls))
$(eval $(call BuildPackage,mbedtls-util))
