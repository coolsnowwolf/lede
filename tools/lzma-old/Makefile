# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=lzma-old
PKG_VERSION:=4.32

PKG_SOURCE:=lzma-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://downloads.openwrt.org/sources/
PKG_HASH:=49053e4bb5e0646a841d250d9cb81f7714f5fff04a133216c4748163567acc3d

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/lzma-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

LIB_DIR=$(HOST_BUILD_DIR)/C/7zip/Compress/LZMA_Lib
ALONE_DIR=$(HOST_BUILD_DIR)/C/7zip/Compress/LZMA_Alone

define Host/Compile
	$(MAKE) -C $(LIB_DIR)
	$(MAKE) -f makefile.gcc -C $(ALONE_DIR)
endef

define Host/Install
	$(INSTALL_DATA) $(LIB_DIR)/liblzma.a $(STAGING_DIR_HOST)/lib/liblzma-old.a
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/lib/liblzma-old.a
endef

$(eval $(call HostBuild))
