#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libpsl
PKG_VERSION:=0.21.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/rockdaboot/libpsl/releases/download/$(PKG_VERSION)
PKG_HASH:=ac6ce1e1fbd4d0254c4ddb9d37f1fa99dec83619c1253328155206b896210d4c

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

define Package/libpsl
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C library to handle the Public Suffix List
  URL:=https://github.com/rockdaboot/libpsl
  DEPENDS:=+libidn2 +libunistring $(INTL_DEPENDS)
  ABI_VERSION:=5
endef

define Package/libpsl/description
  C library to handle the Public Suffix List
endef

MESON_ARGS += \
	-Druntime=libidn2 \
	-Dbuiltin=libidn2

TARGET_LDFLAGS += -liconv

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libpsl.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpsl.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libpsl.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libpsl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpsl.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libpsl))
