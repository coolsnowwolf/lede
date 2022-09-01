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

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=gpl-2.0.txt
PKG_CPE_ID:=cpe:/a:arm:mbed_tls

PKG_CONFIG_DEPENDS := \
	CONFIG_LIBMBEDTLS_DEBUG_C \
	CONFIG_LIBMBEDTLS_HAVE_ARMV8CE_AES \
	CONFIG_LIBMBEDTLS_HAVE_SSE2 \
	CONFIG_LIBMBEDTLS_HKDF_C

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
endef

define Package/libmbedtls/config
config LIBMBEDTLS_DEBUG_C
	depends on PACKAGE_libmbedtls
	bool "Enable debug functions"
	default n
	help
	 This option enables mbedtls library's debug functions.

	 It increases the uncompressed libmbedtls binary size
	 by around 60 KiB (for an ARMv5 platform).

	 Usually, you don't need this, so don't select this if you're unsure.

config LIBMBEDTLS_HAVE_ARMV8CE_AES
	depends on PACKAGE_libmbedtls
	bool
	default y
	prompt "Enable use of the ARMv8 Crypto Extensions"
	depends on aarch64 && !TARGET_bcm27xx && !TARGET_bcm4908
	help
	 Use of the ARMv8 Crypto Extensions greatly increase performance
	 (up to 4x faster on AES-GCM while 10x faster on raw AES).

	 Related instructions should be included in all modern Aarch64
	 devices, except some wastes like Broadcom.
	 If you don't sure, say Y here.

config LIBMBEDTLS_HAVE_SSE2
	depends on PACKAGE_libmbedtls
	bool
	default y if !TARGET_x86_legacy && !TARGET_x86_geode
	prompt "Enable use of x86 SSE2 instructions"
	depends on x86_64 || i386
	help
	 Use of SSE2 instructions greatly increase performance (up to
	 3x faster) with a minimum (~0.2%, or 23KB) increase in package
	 size, but it will bring no benefit if your hardware does not
	 support them, such as Geode GX and LX.  In this case you may
	 save 23KB by saying yes here.  AMD Geode NX, and Intel
	 Pentium 4 and above support SSE2.

config LIBMBEDTLS_HKDF_C
	depends on PACKAGE_libmbedtls
	bool "Enable the HKDF algorithm (RFC 5869)"
	default n
	help
	 This option adds support for the Hashed Message Authentication Code
	 (HMAC)-based key derivation function (HKDF).
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

PKG_INSTALL:=1

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS))
ifneq ($(CONFIG_LIBMBEDTLS_HAVE_ARMV8CE_AES),)
  TARGET_CFLAGS := $(filter-out -march=%,$(TARGET_CFLAGS)) -march=armv8-a+crypto
endif

CMAKE_OPTIONS += \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DUSE_SHARED_MBEDTLS_LIBRARY:Bool=ON \
	-DENABLE_TESTING:Bool=OFF \
	-DENABLE_PROGRAMS:Bool=ON

define Build/Configure
	$(Build/Configure/Default)

	awk 'BEGIN { rc = 1 } \
	     /#define MBEDTLS_DEBUG_C/ { $$$$0 = "$(if $(CONFIG_LIBMBEDTLS_DEBUG_C),,// )#define MBEDTLS_DEBUG_C"; rc = 0 } \
	     /#define MBEDTLS_ARMV8CE_AES_C/ { $$$$0 = "$(if $(CONFIG_LIBMBEDTLS_HAVE_ARMV8CE_AES),,// )#define MBEDTLS_ARMV8CE_AES_C"; rc = 0 } \
	     /#define MBEDTLS_HAVE_SSE2/ { $$$$0 = "$(if $(CONFIG_LIBMBEDTLS_HAVE_SSE2),,// )#define MBEDTLS_HAVE_SSE2"; rc = 0 } \
	     { print } \
	     END { exit(rc) }' $(PKG_BUILD_DIR)/include/mbedtls/config.h \
	     >$(PKG_BUILD_DIR)/include/mbedtls/config.h.new && \
	mv $(PKG_BUILD_DIR)/include/mbedtls/config.h.new $(PKG_BUILD_DIR)/include/mbedtls/config.h

	awk 'BEGIN { rc = 1 } \
	     /#define MBEDTLS_HKDF_C/ { $$$$0 = "$(if $(CONFIG_LIBMBEDTLS_HKDF_C),,// )#define MBEDTLS_HKDF_C"; rc = 0 } \
	     { print } \
	     END { exit(rc) }' $(PKG_BUILD_DIR)/include/mbedtls/config.h \
	     >$(PKG_BUILD_DIR)/include/mbedtls/config.h.new && \
	mv $(PKG_BUILD_DIR)/include/mbedtls/config.h.new $(PKG_BUILD_DIR)/include/mbedtls/config.h

	sed -i '/fuzz/d' $(PKG_BUILD_DIR)/programs/CMakeLists.txt
	sed -i '/test/d' $(PKG_BUILD_DIR)/programs/CMakeLists.txt
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
