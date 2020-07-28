#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mbedtls
PKG_VERSION:=2.16.6
PKG_RELEASE:=1
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-gpl.tgz
PKG_SOURCE_URL:=https://tls.mbed.org/download/
PKG_HASH:=80a484df42f32dbe95665cd4b18ce0dd14b6c67dfd561d36d1475802e41eb3ed

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=gpl-2.0.txt
PKG_CPE_ID:=cpe:/a:arm:mbed_tls

PKG_CONFIG_DEPENDS:=CONFIG_LIBMBEDTLS_DEBUG_C

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

CMAKE_OPTIONS += \
	-DUSE_SHARED_MBEDTLS_LIBRARY:Bool=ON \
	-DENABLE_TESTING:Bool=OFF \
	-DENABLE_PROGRAMS:Bool=ON

define Build/Configure
	$(Build/Configure/Default)

	awk 'BEGIN { rc = 1 } \
	     /#define MBEDTLS_DEBUG_C/ { $$$$0 = "$(if $(CONFIG_LIBMBEDTLS_DEBUG_C),,// )#define MBEDTLS_DEBUG_C"; rc = 0 } \
	     { print } \
	     END { exit(rc) }' $(PKG_BUILD_DIR)/include/mbedtls/config.h \
	     >$(PKG_BUILD_DIR)/include/mbedtls/config.h.new && \
	mv $(PKG_BUILD_DIR)/include/mbedtls/config.h.new $(PKG_BUILD_DIR)/include/mbedtls/config.h
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
