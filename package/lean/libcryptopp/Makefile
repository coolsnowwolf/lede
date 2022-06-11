#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcryptopp
PKG_VERSION:=8.5.0
PKG_RELEASE:=1

PKG_SOURCE:=cryptopp$(subst .,,$(PKG_VERSION)).zip
PKG_SOURCE_URL:=https://www.cryptopp.com/
PKG_HASH:=95fc50d59488ebf61a735cce2b2ec2c2561fc682077c7b496273d65a1ed93d9e

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

UNZIP_CMD:=unzip -q -d $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)

define Package/libcryptopp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Free C++ class library of cryptographic schemes
  URL:=https://packages.debian.org/sid/libcrypto++-dev
  DEPENDS:=+libpthread +libstdcpp
endef

define Package/libcryptopp/description
  Crypto++ is library for creating C++ programs which use cryptographic
  algorithms. The library uses a Pipes & Filters architecture with heavy
  use of templates and abstract base classes.
endef

define Build/Prepare
	$(Build/Prepare/Default)
	cd $(PKG_BUILD_DIR); rm -f GNUmakefile GNUmakefile-cross
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
