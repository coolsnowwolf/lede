# Copyright (C) 2019 Valentin Kivachuk <vk18496@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tesseract
PKG_VERSION:=4.1.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/tesseract-ocr/tesseract/tar.gz/$(PKG_VERSION)?
PKG_HASH:=2a66ff0d8595bff8f04032165e6c936389b1e5727c3ce5a27b3e059d218db1cb

PKG_MAINTAINER:=Valentin Kivachuk <vk18496@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

CMAKE_BINARY_SUBDIR:=openwrt-build

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/tesseract
  MENU:=1
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Tesseract Open Source OCR Engine
  URL:=https://github.com/tesseract-ocr/tesseract
  DEPENDS:=+libleptonica +libarchive +libpthread +libstdcpp
endef

TARGET_CFLAGS:=$(filter-out -O%,$(TARGET_CFLAGS)) -O3

CMAKE_OPTIONS += \
	-DBUILD_TRAINING_TOOLS=OFF

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/tesseract $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtesseract.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/cmake
	$(CP) $(PKG_INSTALL_DIR)/usr/cmake/*.cmake $(1)/usr/lib/cmake/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/tesseract.pc $(1)/usr/lib/pkgconfig/
endef

define Package/tesseract/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,tesseract))
