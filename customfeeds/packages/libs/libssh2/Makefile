#
# Copyright (C) 2015-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libssh2
PKG_VERSION:=1.9.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.libssh2.org/download
PKG_HASH:=d5fb8bd563305fd1074dda90bd053fb2d29fc4bce048d182f96eaa466dfadafd

PKG_MAINTAINER:=Jiri Slachta <jiri@slachta.eu>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:libssh2:libssh2

CMAKE_INSTALL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_LIBSSH2_MBEDTLS \
	CONFIG_LIBSSH2_OPENSSL

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libssh2
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=SSH2 library
  URL:=https://www.libssh2.org/
  DEPENDS:=+LIBSSH2_MBEDTLS:libmbedtls +!LIBSSH2_MBEDTLS:libopenssl +zlib
  ABI_VERSION:=1
endef

define Package/libssh2/description
 libssh2 is a client-side C library implementing the SSH2 protocol.
endef

define Package/libssh2/config
if PACKAGE_libssh2

choice
	prompt "Choose crypto backend"
	default LIBSSH2_OPENSSL

	config LIBSSH2_OPENSSL
	bool "openssl"

	config LIBSSH2_MBEDTLS
	bool "mbedtls"
endchoice

endif
endef

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_TESTING=OFF \
	-DENABLE_ZLIB_COMPRESSION=ON \
	-DCLEAR_MEMORY=ON \
	-DCRYPTO_BACKEND=$(if $(CONFIG_LIBSSH2_MBEDTLS),mbedTLS,OpenSSL)

define Package/libssh2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libssh2.so.$(ABI_VERSION)* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libssh2))
