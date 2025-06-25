#
# Copyright (C) 2009-2025 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=erofs-utils
PKG_VERSION:=1.8.9
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://git.kernel.org/pub/scm/linux/kernel/git/xiang/erofs-utils.git
PKG_MIRROR_HASH:=dfeeca4e3b1337cf05d2b40f6f3956601b98fba0b6a8d7363071ae366a2020e1
PKG_SOURCE_DATE:=2025-06-26
PKG_SOURCE_VERSION:=81169bf3cfd26b8f2b3aa3b20da23971168a90a9

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS=\
	--enable-multithreading \
	--enable-lzma \
	--enable-lz4 \
	--disable-fuse \
	--with-uuid

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/mkfs/mkfs.erofs $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/fsck/fsck.erofs $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mkfs.erofs
	rm -f $(STAGING_DIR_HOST)/bin/fsck.erofs
endef

$(eval $(call HostBuild))
