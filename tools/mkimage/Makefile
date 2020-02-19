#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mkimage
PKG_VERSION:=2018.03

PKG_SOURCE:=u-boot-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=\
	https://sources.openwrt.org/ \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=7e7477534409d5368eb1371ffde6820f0f79780a1a1f676161c48442cb303dfd

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/u-boot-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

define Host/Prepare
	$(Host/Prepare/Default)
	mkdir -p $(HOST_BUILD_DIR)/include/config
	touch $(HOST_BUILD_DIR)/include/config/auto.conf
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) \
		HOSTCFLAGS="$(HOST_CFLAGS)" \
		HOSTLDFLAGS="$(HOST_LDFLAGS)" \
		no-dot-config-targets=tools-only \
		CONFIG_MKIMAGE_DTC_PATH=dtc \
		CONFIG_FIT_SIGNATURE=y \
		tools-only
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/tools/mkimage $(STAGING_DIR_HOST)/bin/
	$(CP) $(HOST_BUILD_DIR)/tools/mkenvimage $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mkimage
	rm -f $(STAGING_DIR_HOST)/bin/mkenvimage
endef

$(eval $(call HostBuild))
