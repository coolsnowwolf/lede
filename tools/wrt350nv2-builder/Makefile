# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wrt350nv2-builder
PKG_VERSION:=2.4
PKG_REVISION:=2

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/${PKG_NAME}-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	$(HOSTCC) $(HOST_CFLAGS) -c src/md5.c -o $(HOST_BUILD_DIR)/md5.o
	$(HOSTCC) $(HOST_CFLAGS) -c src/ioapi.c -o $(HOST_BUILD_DIR)/ioapi.o
	$(HOSTCC) $(HOST_CFLAGS) -c src/wrt350nv2-builder.c -o $(HOST_BUILD_DIR)/wrt350nv2-builder.o
	$(HOSTCC) $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $(HOST_BUILD_DIR)/wrt350nv2-builder \
		$(HOST_BUILD_DIR)/wrt350nv2-builder.o $(HOST_BUILD_DIR)/md5.o $(HOST_BUILD_DIR)/ioapi.o
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/wrt350nv2-builder $(STAGING_DIR_HOST)/bin/
# optionally the builder can add a u-boot binary to create a complete webupgrade image, but this is not mandatory
# u-boot images from stock firmware are available at ftp://ftp.maddes.net/openwrt/trunk/orion/stock/binaries/
	[ ! -f "files/u-boot.bin" ] || ( \
		$(INSTALL_DIR) $(STAGING_DIR_HOST)/share/wrt350nv2-builder; \
		$(CP) files/u-boot.bin $(STAGING_DIR_HOST)/share/wrt350nv2-builder/; )
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/wrt350nv2-builder
	rm -f $(STAGING_DIR_HOST)/share/wrt350nv2-builder
endef

$(eval $(call HostBuild))
