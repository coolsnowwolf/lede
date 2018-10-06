# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=squashfs
PKG_CPE_ID:=cpe:/a:phillip_lougher:squashfs
PKG_VERSION:=3.0

PKG_SOURCE:=$(PKG_NAME)$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/squashfs
PKG_HASH:=39dbda43cf118536deb746c7730b468702d514a19f4cfab73b710e32908ddf20
PKG_CAT:=zcat

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)/squashfs-tools \
		CC="$(HOSTCC)" \
		CFLAGS="$(HOST_CFLAGS) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -I." \
		CXX="$(CXX)" \
		LZMAPATH=$(STAGING_DIR_HOST)/lib \
		mksquashfs-lzma unsquashfs-lzma 
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/mksquashfs-lzma $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/unsquashfs-lzma $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mksquashfs-lzma
	rm -f $(STAGING_DIR_HOST)/bin/unsquashfs-lzma
endef

$(eval $(call HostBuild))
