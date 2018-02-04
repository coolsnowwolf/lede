#
# Copyright (C) 2008-2013 OpenWrt.org
# Copyright (C) 2017 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libunwind
PKG_VERSION:=1.2.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SAVANNAH/$(PKG_NAME)
PKG_HASH:=3f3ecb90e28cbe53fba7a4a27ccce7aad188d3210bb1964a923a731a27a75acb
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_LICENSE:=X11
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:libunwind_project:libunwind

PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

PKG_SSP:=0
include $(INCLUDE_DIR)/package.mk

define Package/libunwind
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=The libunwind project
  URL:=http://www.nongnu.org/libunwind/
  DEPENDS:=@(mips||mipsel||powerpc||i386||x86_64)
endef

define Package/libunwind/description
  Libunwind defines a portable and efficient C programming interface (API) to determine the call-chain of a program.
endef

CONFIGURE_ARGS += \
	--disable-documentation \
	--enable-minidebuginfo=no \


define Package/libunwind/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunwin*.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libunwin*.so* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,libunwind))
