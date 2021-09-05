#
# Copyright (C) 2020 Linos Giannopoulos
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libfido2
PKG_VERSION:=1.6.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/Yubico/libfido2/tar.gz/$(PKG_VERSION)?
PKG_HASH:=6aed47aafd22be49c38f9281fb88ccd08c98678d9b8c39cdc87d1bb3ea2c63e4

PKG_FORTIFY_SOURCE:=0
CMAKE_INSTALL:=1

TARGET_CFLAGS += -Wno-error=overflow -Wno-error=sign-conversion

PKG_MAINTAINER:=Linos Giannopoulos <linosgian00+openwrt@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libfido2
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=FIDO2 Library
  URL:=https://github.com/Yubico/libfido2
  ABI_VERSION:=1
  DEPENDS += +libcbor +libopenssl +libudev
endef

define Package/libfido2/description
 libfido2 provides library functionality and command-line tools to communicate with a FIDO device over USB, and to verify attestation and assertion signatures.

 libfido2 supports the FIDO U2F (CTAP 1) and FIDO 2.0 (CTAP 2) protocols.
endef

CMAKE_OPTIONS += \
	-DBUILD_EXAMPLES=OFF \
	-DBUILD_MANPAGES=OFF \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_STATIC_LIBS=OFF \
	-DBUILD_TOOLS=OFF \
	-DFUZZ=OFF \
	-DLIBFUZZER=OFF \
	-DUSE_HIDAPI=OFF

define Package/libfido2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfido2.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libfido2))
