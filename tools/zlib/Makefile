#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zlib
PKG_VERSION:=1.2.12
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/madler/zlib
PKG_MIRROR_HASH:=a162fc219763635f0c1591ec515d4b08684e4b0bfb4b1c8e65e4eab18d597c27
PKG_SOURCE_VERSION:=21767c654d31d2dccdde4330529775c6c5fd5389

PKG_LICENSE:=Zlib
PKG_LICENSE_FILES:=README
PKG_CPE_ID:=cpe:/a:gnu:zlib

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

HOST_CFLAGS +=-fPIC

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/libz.a $(STAGING_DIR_HOST)/lib/
	$(CP) $(HOST_BUILD_DIR)/zconf.h $(STAGING_DIR_HOST)/include/
	$(CP) $(HOST_BUILD_DIR)/zlib.h $(STAGING_DIR_HOST)/include/
	$(CP) $(HOST_BUILD_DIR)/zlib.pc $(STAGING_DIR_HOST)/lib/pkgconfig/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/lib/libz.a
	rm -f $(STAGING_DIR_HOST)/include/zconf.h
	rm -f $(STAGING_DIR_HOST)/include/zlib.h
	rm -f $(STAGING_DIR_HOST)/lib/pkgconfig//zlib.pc
endef

$(eval $(call HostBuild))
