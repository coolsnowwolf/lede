#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=squashfs4
PKG_CPE_ID:=cpe:/a:phillip_lougher:squashfs
PKG_VERSION:=4.6.1
PKG_RELEASE=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/plougher/squashfs-tools
PKG_SOURCE_DATE:=2023-03-26
PKG_SOURCE_VERSION:=d8cb82d9840330f9344ec37b992595b5d7b44184
PKG_MIRROR_HASH:=e84026de1ab187f3f76d1b781a29259d818f887e1651225f850a62d6f90b1b9e

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	+$(HOST_MAKE_VARS) \
	$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/squashfs-tools \
		XZ_SUPPORT=1 \
		LZMA_XZ_SUPPORT=1 \
		XZ_EXTENDED_OPTIONS=1 \
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
