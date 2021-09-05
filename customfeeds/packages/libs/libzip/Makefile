#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libzip
PKG_VERSION:=1.7.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://libzip.org/download/
PKG_HASH:=a60473ffdb7b4260c08bfa19c2ccea0438edac11193c3afbbb1f17fbcf6c6132

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libzip/Default
  TITLE:=libzip ($(2))
  URL:=https://libzip.org/
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Compression
  DEPENDS:=+zlib $(3)
  VARIANT:=$(1)
  PROVIDES:=libzip
endef

define Package/libzip-$(BUILD_VARIANT)/description
 A C library for reading, creating, and modifying zip archives.
endef

Package/libzip-nossl=$(call Package/libzip/Default,nossl,w/o encryption support)
Package/libzip-openssl=$(call Package/libzip/Default,openssl,OpenSSL,+PACKAGE_libzip-openssl:libopenssl)
Package/libzip-gnutls=$(call Package/libzip/Default,gnutls,GnuTLS,+PACKAGE_libzip-gnutls:libgnutls)
Package/libzip-mbedtls=$(call Package/libzip/Default,mbedtls,mbedTLS,+PACKAGE_libzip-mbedtls:libmbedtls)

define Package/zipcmp
  TITLE:=zipcmp
  URL:=https://libzip.org/
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Compression
  DEPENDS:=+libzip +USE_MUSL:musl-fts
endef

define Package/zipcmp/description
 This package contains the command line tool zipcmp from libzip.
endef

define Package/zipmerge
  TITLE:=zipmerge
  URL:=https://libzip.org/
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Compression
  DEPENDS:=+libzip
endef

define Package/zipmerge/description
 This package contains the command line tool zipmerge from libzip.
endef

define Package/ziptool
  TITLE:=ziptool
  URL:=https://libzip.org/
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Compression
  DEPENDS:=+libzip
endef

define Package/ziptool/description
 This package contains the command line tool ziptool from libzip.
endef

CMAKE_OPTIONS += -DENABLE_COMMONCRYPTO=OFF
ifeq ($(BUILD_VARIANT),gnutls)
  CMAKE_OPTIONS += -DENABLE_GNUTLS=ON
else
  CMAKE_OPTIONS += -DENABLE_GNUTLS=OFF
endif
ifeq ($(BUILD_VARIANT),openssl)
  CMAKE_OPTIONS += -DENABLE_OPENSSL=ON
else
  CMAKE_OPTIONS += -DENABLE_OPENSSL=OFF
endif
ifeq ($(BUILD_VARIANT),mbedtls)
  CMAKE_OPTIONS += -DENABLE_MBEDTLS=ON
else
  CMAKE_OPTIONS += -DENABLE_MBEDTLS=OFF
endif

CMAKE_OPTIONS += -DENABLE_BZIP2=OFF
CMAKE_OPTIONS += -DENABLE_LZMA=OFF
CMAKE_OPTIONS += -DBUILD_REGRESS=OFF
CMAKE_OPTIONS += -DBUILD_EXAMPLES=OFF
CMAKE_OPTIONS += -DBUILD_DOC=OFF
CMAKE_OPTIONS += -DBUILD_TOOLS=ON

define Package/libzip-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libzip.so.* $(1)/usr/lib/
endef

define Package/zipcmp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/zipcmp $(1)/usr/bin/
endef

define Package/zipmerge/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/zipmerge $(1)/usr/bin/
endef

define Package/ziptool/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ziptool $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libzip-gnutls))
$(eval $(call BuildPackage,libzip-mbedtls))
$(eval $(call BuildPackage,libzip-openssl))
$(eval $(call BuildPackage,libzip-nossl))
$(eval $(call BuildPackage,zipcmp))
$(eval $(call BuildPackage,zipmerge))
$(eval $(call BuildPackage,ziptool))
