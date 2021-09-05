#
# libnopoll - Makefile for noPoll C WebSocket library
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nopoll
PKG_VERSION:=0.4.6
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://codeload.github.com/ASPLes/nopoll/tar.gz/$(PKG_VERSION)?
PKG_SOURCE:=nopoll-$(PKG_VERSION).tar.gz
PKG_HASH:=16eae3885d572495ba1f22d85fa9032139c761fb21ef1d80205039acd3143f5a
PKG_BUILD_DIR:=$(BUILD_DIR)/nopoll-$(PKG_VERSION)

PKG_MAINTAINER:=John Clark <inindev@gmail.com>
PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libnopoll
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libopenssl
  TITLE:=noPoll C WebSocket Library
  URL:=https://www.aspl.es/nopoll
endef

define Package/libnopoll/description
  This package contains the noPoll C WebSocket library.
endef

TARGET_LDFLAGS += -lcrypto -lpthread

CONFIGURE_ARGS += \
	--enable-nopoll-log=no \
	--enable-nopoll-doc=no

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{lib,include}
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnopoll.{a,so*} $(1)/usr/lib/
endef

define Package/libnopoll/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnopoll.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnopoll))
