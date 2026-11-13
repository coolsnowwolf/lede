#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
PKG_VERSION:=2.3.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://infraroot.at/pub/mtd/
PKG_HASH:=03d9dc58ad10ea3549d9528f6b17a44d8944e18e96c0f31474f9f977078b83dc
PKG_CPE_ID:=cpe:/a:mtd-utils_project:mtd-utils

PKG_FIXUP:=autoreconf

HOST_BUILD_PARALLEL:=1
HOST_CONFIGURE_CACHE:=1

include $(INCLUDE_DIR)/host-build.mk

ifneq ($(HOST_OS),Linux)
  HOST_CFLAGS += \
	-I$(CURDIR)/include \
	-Dloff_t=off_t \
	-D__BYTE_ORDER=BYTE_ORDER \
	-include endian.h \
	-DNO_NATIVE_SUPPORT \
	-include fls.h
endif

HOST_CONFIGURE_ARGS+= \
	--without-tests \
	--without-crypto \
	--without-xattr \
	--without-zstd \
	--without-lzo \
	--with-lzma

HOST_MAKE_FLAGS += \
	PROGRAMS="mkfs.jffs2 ubinize mkfs.ubifs"

define Host/Install
	$(CP) \
		$(HOST_BUILD_DIR)/mkfs.jffs2 \
		$(HOST_BUILD_DIR)/mkfs.ubifs \
		$(HOST_BUILD_DIR)/ubinize \
		$(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/{mkfs.jffs2,mkfs.ubifs,ubinize}
endef

$(eval $(call HostBuild))
