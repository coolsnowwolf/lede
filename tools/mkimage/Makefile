#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mkimage
PKG_VERSION:=2024.07

PKG_SOURCE:=u-boot-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	https://mirror.cyberbits.eu/u-boot \
	https://ftp.denx.de/pub/u-boot \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=f591da9ab90ef3d6b3d173766d0ddff90c4ed7330680897486117df390d83c8f

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/u-boot-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

define Host/Configure
	$(MAKE) -C $(HOST_BUILD_DIR) \
		HOSTCFLAGS="$(HOST_CFLAGS)" \
		HOSTLDFLAGS="$(HOST_LDFLAGS)" \
		PKG_CONFIG_EXTRAARGS="--static" \
		V=$(if $(findstring c,$(OPENWRT_VERBOSE)),1) \
		tools-only_config

	sed -i 's/CONFIG_TOOLS_MKEFICAPSULE=y/# CONFIG_TOOLS_MKEFICAPSULE is not set/' $(HOST_BUILD_DIR)/.config
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) \
		HOSTCFLAGS="$(HOST_CFLAGS)" \
		HOSTLDFLAGS="$(HOST_LDFLAGS)" \
		PKG_CONFIG_EXTRAARGS="--static" \
		V=$(if $(findstring c,$(OPENWRT_VERBOSE)),1) \
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
