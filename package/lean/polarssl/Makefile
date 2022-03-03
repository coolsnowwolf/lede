#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=polarssl
SRC_PKG_NAME:=mbedtls
PKG_VERSION:=1.3.16
PKG_RELEASE:=1
PKG_USE_MIPS16:=0

PKG_SOURCE:=$(SRC_PKG_NAME)-$(PKG_VERSION)-gpl.tgz
PKG_SOURCE_URL:=https://polarssl.org/download/
PKG_MD5SUM:=66ebbbf67e8a9463041846822b0a1692

PKG_BUILD_DIR:=$(BUILD_DIR)/$(SRC_PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/polarssl/Default
  SUBMENU:=SSL
  TITLE:=Embedded SSL
  URL:=http://polarssl.org/
endef

define Package/polarssl/Default/description
The aim of the PolarSSL project is to provide a quality, open-source
cryptographic library written in C and targeted at embedded systems.
endef

define Package/libpolarssl
$(call Package/polarssl/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= (library)
  ABI_VERSION:=$(PKG_VERSION)-$(PKG_RELEASE)
endef

define Package/libpolarssl/description
$(call Package/polarssl/Default/description)
This package contains the PolarSSL library.
endef

PKG_INSTALL:=1

CMAKE_OPTIONS += \
	-DCMAKE_BUILD_TYPE:String="Release" \
	-DUSE_SHARED_MBEDTLS_LIBRARY:Bool=ON \
	-DENABLE_TESTING:Bool=OFF \
	-DENABLE_PROGRAMS:Bool=OFF \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/polarssl $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmbedtls.so.* $(1)/usr/lib/
	$(LN) libmbedtls.so.$(PKG_VERSION) $(1)/usr/lib/libpolarssl.so
endef

define Package/libpolarssl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmbedtls.so.* $(1)/usr/lib/
	$(LN) libmbedtls.so.$(PKG_VERSION) $(1)/usr/lib/libpolarssl.so
endef

$(eval $(call BuildPackage,libpolarssl))
