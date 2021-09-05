# 
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=liboil
PKG_VERSION:=0.3.17
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://liboil.freedesktop.org/download/
PKG_HASH:=105f02079b0b50034c759db34b473ecb5704ffa20a5486b60a8b7698128bfc69

PKG_LICENSE:=FREE
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/liboil
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=simple functions optimized for various CPUs
  URL:=http://liboil.freedesktop.org/wiki/
  MAINTAINER:=W. Michael Petullo <mike@flyn.org>
  DEPENDS:=+librt $(ICONV_DEPENDS) $(INTL_DEPENDS)
endef

define Package/liboil/description
  Liboil is a library of simple functions that are optimized for various CPUs.
  These functions are generally loops implementing simple algorithms, such as
  converting an array of N integers to floating-point numbers or multiplying
  and summing an array of N numbers. Such functions are candidates for significant
  optimization using various techniques, especially by using extended instructions
  provided by modern CPUs (Altivec, MMX, SSE, etc.).
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \

TARGET_CFLAGS += $(FPIC) $(if $(ICONV_FULL),-liconv) $(if $(INTL_FULL),-lintl)

# XXX: VFP_CFLAGS is set to '-mfpu=vfp' on arm by configure, but that breaks 
# final linking stages, so override it until we find why
MAKE_FLAGS += \
	VFP_CFLAGS="" \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/liboil-0.3/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboil-0.3.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/liboil-0.3.pc $(1)/usr/lib/pkgconfig/
endef

define Package/liboil/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboil-0.3.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,liboil))
