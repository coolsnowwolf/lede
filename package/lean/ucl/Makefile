#
# Copyright (C) 2019-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=ucl
PKG_VERSION:=1.03
PKG_RELEASE:=3

PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.oberhumer.com/opensource/ucl/download
PKG_HASH:=b865299ffd45d73412293369c9754b07637680e5c826915f097577cd27350348

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

HOST_FIXUP:=autoreconf
HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

HOST_BUILD_PREFIX:=$(STAGING_DIR_HOST)

HOST_CFLAGS += -std=gnu89
TARGET_CFLAGS += -std=gnu89

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared \
	--disable-asm

define Package/libucl
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Portable lossless data compression library
  URL:=http://www.oberhumer.com/opensource/ucl/
endef

define Package/libucl/description
UCL is a portable lossless data compression library written in ANSI C. UCL
implements a number of compression algorithms that achieve an excellent
compression ratio while allowing *very* fast decompression. Decompression
requires no additional memory.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/ucl
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ucl/ucl{,conf}.h $(1)/usr/include/ucl/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libucl.{a,so*} $(1)/usr/lib/
endef

define Package/libucl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libucl.so.* $(1)/usr/lib/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libucl))
