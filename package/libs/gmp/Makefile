#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gmp
PKG_VERSION:=6.3.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/gmp/
PKG_HASH:=e56fd59d76810932a0555aa15a14b61c16bed66110d3c75cc2ac49ddaa9ab24c

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_LICENSE:=GPL-2.0-or-later
PKG_CPE_ID:=cpe:/a:gmplib:gmp

PKG_BUILD_FLAGS:=no-mips16

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/libgmp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU multiprecision arithmetic library
  URL:=https://gmplib.org/
  ABI_VERSION:=10
endef

define Package/libgmp/description
	GMP is a free library for arbitrary precision arithmetic, operating on
	signed integers, rational numbers, and floating point numbers.
endef

TARGET_CFLAGS += $(FPIC)
CONFIGURE_VARS += CC="$(TARGET_CROSS)gcc"
CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--without-readline \
	--disable-fft \

define Build/Compile
	$(call Build/Compile/Default, \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)" \
		all \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gmp* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgmp.{a,so*} $(1)/usr/lib/
endef

define Package/libgmp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgmp.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libgmp))
$(eval $(call HostBuild))
