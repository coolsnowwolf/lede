#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=squashfskit4
PKG_VERSION:=4.14
PKG_RELEASE:=3
PKG_SOURCE:=squashfskit-v$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/squashfskit/squashfskit/releases/download/v$(PKG_VERSION)/
PKG_HASH:=5761aaa3aedc4f7112b708367d891c9abdc1ffea972e3fe47923ddba23984d95

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/squashfskit-v$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	+$(HOST_MAKE_VARS) \
	$(MAKE) -C $(HOST_BUILD_DIR)/squashfs-tools \
		XZ_SUPPORT=1 \
		LZMA_XZ_SUPPORT=1 \
		XATTR_SUPPORT=1 \
		LZMA_LIB="$(STAGING_DIR_HOST)/lib/liblzma.a" \
		EXTRA_CFLAGS="-I$(STAGING_DIR_HOST)/include" \
		mksquashfs unsquashfs
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/mksquashfs $(STAGING_DIR_HOST)/bin/mksquashfs4
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/unsquashfs $(STAGING_DIR_HOST)/bin/unsquashfs4
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mksquashfs4
	rm -f $(STAGING_DIR_HOST)/bin/unsquashfs4
endef

$(eval $(call HostBuild))
