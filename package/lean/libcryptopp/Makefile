#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcryptopp
PKG_VERSION:=5.6.4
PKG_RELEASE:=2

PKG_SOURCE:=cryptopp$(subst .,,$(PKG_VERSION)).zip
PKG_SOURCE_URL:=http://www.cryptopp.com/
PKG_MD5SUM:=4ee7e5cdd4a45a14756c169eaf2a77fc

PKG_USE_NINJA:=0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_NAME)-$(PKG_VERSION)

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libcryptopp
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpthread +libstdcpp
  TITLE:=Free C++ class library of cryptographic schemes
  URL:=http://packages.debian.org/sid/libcrypto++-dev
endef

define Package/libcryptopp/description
  Crypto++ is library for creating C++ programs which use cryptographic algorithms.
  The library uses a Pipes & Filters architecture with heavy use of templates and
  abstract base classes.
endef

#CONFIGURE_ARGS += \
#	--enable-static \
#	--enable-shared

#TARGET_LDFLAGS += \
#	-lpthread

CMAKE_OPTIONS += -DCMAKE_CXX_FLAGS="$(FPIC)"
CMAKE_OPTIONS += -DBUILD_STATIC=ON
CMAKE_OPTIONS += -DBUILD_SHARED=ON

UNPACK_CMD=cd $(PKG_BUILD_DIR) && unzip "$(DL_DIR)/$(PKG_SOURCE)"

define Build/Prepare
	$(Build/Prepare/Default)
	cd $(PKG_BUILD_DIR) && rm -f GNUmakefile GNUmakefile-cross
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/cryptopp
	$(CP) $(PKG_INSTALL_DIR)/usr/include/cryptopp/* $(1)/usr/include/cryptopp/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcryptopp.{a,so*} $(1)/usr/lib/
endef

define Package/libcryptopp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcryptopp.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcryptopp))
