#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mbedtls
PKG_VERSION:=2.28.1
PKG_RELEASE:=$(AUTORELEASE)
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ARMmbed/mbedtls/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=6797a7b6483ef589deeab8d33d401ed235d7be25eeecda1be8ddfed406d40ff4

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=gpl-2.0.txt
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
  CONFIG_MBEDTLS_NIST_KW_C \
  CONFIG_MBEDTLS_RIPEMD160_C \
  CONFIG_MBEDTLS_RSA_NO_CRT \
  CONFIG_MBEDTLS_XTEA_C

MBEDTLS_BUILD_OPTS= \
  $(MBEDTLS_BUILD_OPTS_CURVES) \
  $(MBEDTLS_BUILD_OPTS_CIPHERS) \
  CONFIG_MBEDTLS_CERTS_C \
  CONFIG_MBEDTLS_CIPHER_MODE_OFB \
  CONFIG_MBEDTLS_CIPHER_MODE_XTS \
  CONFIG_MBEDTLS_DEBUG_C \
  CONFIG_MBEDTLS_ENTROPY_FORCE_SHA256 \
  CONFIG_MBEDTLS_HKDF_C \
  CONFIG_MBEDTLS_PLATFORM_C \
  CONFIG_MBEDTLS_SELF_TEST \
  CONFIG_MBEDTLS_SSL_RENEGOTIATION \
  CONFIG_MBEDTLS_SSL_TRUNCATED_HMAC \
  CONFIG_MBEDTLS_VERSION_C \
  CONFIG_MBEDTLS_VERSION_FEATURES

PKG_CONFIG_DEPENDS := $(MBEDTLS_BUILD_OPTS)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mbedtls/Default
  TITLE:=Embedded SSL
  URL:=https://tls.mbed.org
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
  ABI_VERSION:=12
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

define Build/Configure
	$(call Build/Configure/Default)

	sed -i '/fuzz/d' $(PKG_BUILD_DIR)/programs/CMakeLists.txt
	sed -i '/test/d' $(PKG_BUILD_DIR)/programs/CMakeLists.txt
endef

define Build/Prepare
       $(call Build/Prepare/Default)

       $(if $(strip $(foreach opt,$(MBEDTLS_BUILD_OPTS),$($(opt)))),
	 $(foreach opt,$(MBEDTLS_BUILD_OPTS),
	 $(PKG_BUILD_DIR)/scripts/config.py \
	 -f $(PKG_BUILD_DIR)/include/mbedtls/config.h \
	 $(if $($(opt)),set,unset) $(patsubst CONFIG_%,%,$(opt))),)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/mbedtls $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.a $(1)/usr/lib/
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
