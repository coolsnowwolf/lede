#
# Copyright (C) 2018 Othmar Truniger
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libfmt
PKG_VERSION:=8.0.0
PKG_RELEASE:=1

PKG_SOURCE_NAME:=fmt
PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/fmtlib/$(PKG_SOURCE_NAME)/tar.gz/$(PKG_VERSION)?
PKG_HASH:=7bce0e9e022e586b178b150002e7c2339994e3c2bbe44027e9abb0d60f9cce83
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Othmar Truniger <github@truniger.ch>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.rst

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=ON
CMAKE_OPTIONS += -DFMT_DOC=OFF
CMAKE_OPTIONS += -DFMT_TEST=OFF

define Package/libfmt
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Small, safe and fast formatting library
  URL:=https://github.com/fmtlib/fmt
  DEPENDS:=+libstdcpp
endef

define Package/libfmt/description
  fmt is an open-source formatting library for C++.
  It can be used as a safe alternative to printf or as a fast alternative to IOStreams.
endef

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/fmt.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/fmt.pc
endef

define Package/libfmt/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfmt.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libfmt))
